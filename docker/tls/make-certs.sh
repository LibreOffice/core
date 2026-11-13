#!/bin/sh
# Makes the certificates that let Collabora Online reach a writing aid
# service over HTTPS: a private certificate authority, and a certificate it
# signs for the service's host name, the one argument. It also writes
# ca-bundle.crt, the usual certificate authorities with the private one
# added, which Collabora Online is given as its own bundle so it can verify
# the service.
#
#   make-certs.sh languagetool
#
# The compose.yaml files of the writing aids run this once before the
# services start, with the service's directory mounted at /service, in an
# image that has openssl and the usual certificate authorities at
# /etc/ssl/certs/ca-certificates.crt. The certificates go to its certs
# directory, and belong to whoever owns the service's directory, not to the
# container's root. Certificates already there are kept: delete the
# directory to have new ones made, or put your own <host>.crt, <host>.key
# and the ca.crt that signed them there instead.

set -e

host="$1"
if [ -z "$host" ]; then
    echo "usage: $0 <host name>" >&2
    exit 1
fi

mkdir -p /service/certs
cd /service/certs

if [ ! -f "$host.crt" ] || [ ! -f "$host.key" ] || [ ! -f ca.crt ]; then
    openssl req -x509 -newkey rsa:3072 -nodes -days 3650 -sha256 \
        -subj "/CN=Collabora Online $host CA" \
        -addext "basicConstraints=critical,CA:TRUE" \
        -addext "keyUsage=critical,keyCertSign,cRLSign" \
        -keyout ca.key -out ca.crt

    openssl req -newkey rsa:3072 -nodes -sha256 -subj "/CN=$host" \
        -keyout "$host.key" -out "$host.csr"

    cat > "$host.ext" <<EOF
basicConstraints=critical,CA:FALSE
keyUsage=critical,digitalSignature,keyEncipherment
extendedKeyUsage=serverAuth
subjectAltName=DNS:$host
EOF
    openssl x509 -req -in "$host.csr" -CA ca.crt -CAkey ca.key -CAcreateserial \
        -days 3650 -sha256 -extfile "$host.ext" -out "$host.crt"

    # The authority signs nothing else: its key is not kept.
    rm -f ca.key ca.srl "$host.csr" "$host.ext"
    chmod 0644 ca.crt "$host.crt"
    chmod 0600 "$host.key"
fi

cat /etc/ssl/certs/ca-certificates.crt ca.crt > ca-bundle.crt
chmod 0644 ca-bundle.crt

chown -R "$(stat -c %u:%g /service)" /service/certs
