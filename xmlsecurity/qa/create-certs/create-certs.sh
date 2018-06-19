#!/bin/bash -e
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# The purpose of this script is to create certificates to be able to test ODF signing code.
#
# What it does:
#
# 1) Create a test root CA.
#
# 2) Create an intermediate CA, as signing certificates with a root CA is
# considered unsafe.
#
# 3) Create two client certificates that can be used to sign ODF documents in
# LibreOffice.
#
# References:
#
# <https://jamielinux.com/docs/openssl-certificate-authority/> for most of the
# commands.
#
# <https://www.sslshopper.com/article-most-common-openssl-commands.html> for
# the PKCS#12 export of self-signed certificates
#

root="$PWD"

if [ -d "$root/ca" ]; then
    echo "'ca' directory already exists in $root, please remove it before running this script."
    exit 1
fi

if [ -z "$SSLPASS" ]; then
    # Unless specified otherwise, we'll use this as a password everywhere.
    export SSLPASS="xmlsecurity"
fi

# 1) Create the root pair.

mkdir "$root/ca"

cd "$root/ca"
mkdir certs crl newcerts private
chmod 700 private
touch index.txt
echo 1000 > serial

sed "s|@ROOT@|$root|g" "$root/templates/root.cnf" > "$root/ca/openssl.cnf"

# Create the root key.
cd "$root/ca"
openssl genrsa -aes256 -out private/ca.key.pem -passout env:SSLPASS 4096
chmod 400 private/ca.key.pem

# Create the root certificate.
cd "$root/ca"
openssl req -config openssl.cnf \
    -key private/ca.key.pem \
    -new -x509 -days 36500 -sha256 -extensions v3_ca \
    -out certs/ca.cert.pem \
    -passin env:SSLPASS \
    -subj '/C=UK/ST=England/O=Xmlsecurity Test/CN=Xmlsecurity Test Root CA'
chmod 444 certs/ca.cert.pem

# 2) Create the intermediate pair.

# Prepare the directory.
mkdir "$root/ca/intermediate"
cd "$root/ca/intermediate"
mkdir certs crl csr newcerts private
chmod 700 private
touch index.txt
echo 1000 > serial

# crlnumber is used to keep track of certificate revocation lists.
echo 1000 > "$root/ca/intermediate/crlnumber"

# Copy the intermediate CA configuration file.
sed "s|@ROOT@|$root|g" "$root/templates/intermediate.cnf" > "$root/ca/intermediate/openssl.cnf"

# Create the intermediate key.

cd "$root/ca"
openssl genrsa -aes256 \
    -out intermediate/private/intermediate.key.pem \
    -passout env:SSLPASS 4096
chmod 400 intermediate/private/intermediate.key.pem

# Create the intermediate certificate.

# Intermediate key.
cd "$root/ca"
openssl req -config intermediate/openssl.cnf -new -sha256 \
    -key intermediate/private/intermediate.key.pem \
    -out intermediate/csr/intermediate.csr.pem \
    -passin env:SSLPASS \
    -subj '/C=UK/ST=England/O=Xmlsecurity Test/CN=Xmlsecurity Intermediate Root CA'

# The certificate itself.
openssl ca -batch -config openssl.cnf -extensions v3_intermediate_ca \
    -days 36500 -notext -md sha256 \
    -in intermediate/csr/intermediate.csr.pem \
    -passin env:SSLPASS \
    -out intermediate/certs/intermediate.cert.pem
chmod 444 intermediate/certs/intermediate.cert.pem

# Create the certificate chain file.
cat intermediate/certs/intermediate.cert.pem \
    certs/ca.cert.pem > intermediate/certs/ca-chain.cert.pem
chmod 444 intermediate/certs/ca-chain.cert.pem

# 3) Create the real certificate.

for i in Alice Bob
do
    # Create a key.
    cd "$root/ca"
    openssl genrsa -aes256 \
        -out intermediate/private/example-xmlsecurity-${i}.key.pem \
        -passout env:SSLPASS 2048
    chmod 400 intermediate/private/example-xmlsecurity-${i}.key.pem

    # Create a certificate signing request (CSR).

    cd "$root/ca"
    openssl req -config intermediate/openssl.cnf \
        -key intermediate/private/example-xmlsecurity-${i}.key.pem \
        -new -sha256 -out intermediate/csr/example-xmlsecurity-${i}.csr.pem \
        -passin env:SSLPASS \
        -subj "/C=UK/ST=England/O=Xmlsecurity Test/CN=Xmlsecurity Test example ${i}"

    # To create a certificate, use the intermediate CA to sign the CSR.
    cd "$root/ca"
    # usr_cert: the cert will be used for signing.
    openssl ca -batch -config intermediate/openssl.cnf \
        -extensions usr_cert -days 36500 -notext -md sha256 \
        -in intermediate/csr/example-xmlsecurity-${i}.csr.pem \
        -passin env:SSLPASS \
        -out intermediate/certs/example-xmlsecurity-${i}.cert.pem
    chmod 444 intermediate/certs/example-xmlsecurity-${i}.cert.pem

    # Export it in PKCS#12 format.
    openssl pkcs12 -export \
        -out ./intermediate/private/example-xmlsecurity-${i}.cert.p12 \
        -passout env:SSLPASS \
        -inkey intermediate/private/example-xmlsecurity-${i}.key.pem \
        -passin env:SSLPASS \
        -in intermediate/certs/example-xmlsecurity-${i}.cert.pem \
        -certfile intermediate/certs/ca-chain.cert.pem \
        -CSP "Microsoft Enhanced RSA and AES Cryptographic Provider"
done

echo
echo "Authority certificate is at: <$root/ca/intermediate/certs/ca-chain.cert.pem>."
echo "To be able to import it in Windows, rename the '.pem' extension to '.cer'."
for i in Alice Bob
do
    echo "Signing certificate is at <$root/ca/intermediate/private/example-xmlsecurity-${i}.cert.p12>."
done

# vim:set shiftwidth=4 expandtab:
