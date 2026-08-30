#!/bin/sh
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Publish a Collabora Online image the way ZenDiS publishes their hardened
# base images: push, then attach a cosign signature and attestations as
# sidecar tags (sha256-<digest>.sig / .att) on every per-platform manifest
# digest. Registry-agnostic: works for Docker Hub, registry.opencode.de or a
# private registry alike.
#
# Per platform digest this attests:
#   - the image's own CycloneDX 1.6 SBOM   (from /usr/share/sbom/ inside it)
#   - the SPDX 3.0.1 aggregate SBOM        (ditto)
#   - a trivy vulnerability scan, SARIF    (if trivy is installed)
#   - the CycloneDX VEX document           (docker/from-packages/vex/)
#   - SLSA provenance v1                   (built from git metadata)
#
# Usage:
#   publish.sh [-k cosign-key-ref] [-m manifest-list-tag] [-n] image-ref...
#
#   -k  cosign key reference: a file path, env://COSIGN_KEY, or a KMS URI
#       (default: env://COSIGN_KEY; COSIGN_PASSWORD is honoured by cosign).
#       NOTE: docker/from-packages/secret_key is the apt repository secret,
#       NOT a signing key - never pass it here. The private counterpart of
#       docker/cosign.pub is kept outside this repository.
#   -m  additionally assemble the given refs into a multi-arch manifest list
#       under this tag (docker buildx imagetools create) and push it
#   -n  no-push: the refs are already pushed, only sign and attest
#
# Consumers verify with:
#   cosign verify              --key docker/cosign.pub <ref>
#   cosign verify-attestation  --key docker/cosign.pub --type cyclonedx <ref>

set -eu

usage() { sed -n '/^# Usage:/,/^$/s/^# \{0,1\}//p' "$0"; exit 1; }

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
VEX_FILE="$SCRIPT_DIR/../vex/cool.vex.json"

KEY="env://COSIGN_KEY"
MANIFEST_TAG=
PUSH=yes
while getopts k:m:nh opt; do
    case "$opt" in
        k) KEY="$OPTARG" ;;
        m) MANIFEST_TAG="$OPTARG" ;;
        n) PUSH= ;;
        *) usage ;;
    esac
done
shift $((OPTIND - 1))
[ $# -ge 1 ] || usage

command -v cosign >/dev/null || { echo "publish.sh: cosign not found" >&2; exit 1; }

case "$KEY" in
    *secret_key*)
        echo "publish.sh: refusing '$KEY': secret_key is the apt repository" \
             "secret, not a signing key" >&2
        exit 1 ;;
esac

WORKDIR=$(mktemp -d)
trap 'rm -rf "$WORKDIR"' EXIT

# SLSA provenance v1 predicate from the local git checkout and build inputs.
make_provenance() {
    ref="$1"
    commit=$(git -C "$SCRIPT_DIR" rev-parse HEAD 2>/dev/null || echo unknown)
    cat > "$WORKDIR/provenance.json" <<EOF
{
  "buildDefinition": {
    "buildType": "https://collaboraonline.github.io/build-types/docker-from-packages@v1",
    "externalParameters": {
      "source": "https://github.com/CollaboraOnline/online",
      "revision": "${commit}",
      "dockerfile": "docker/from-packages/Dockerfile",
      "imageRef": "${ref}"
    }
  },
  "runDetails": {
    "builder": { "id": "https://collaboraoffice.com/docker-publisher" },
    "metadata": { "finishedOn": "$(date -u +%Y-%m-%dT%H:%M:%SZ)" }
  }
}
EOF
    echo "$WORKDIR/provenance.json"
}

# Copy the in-image SBOMs out of the arch image at the given digest.
extract_sboms() {
    pinned="$1"
    docker pull -q "$pinned" >/dev/null
    container=$(docker create "$pinned")
    docker cp -q "$container:/usr/share/sbom/collabora-online.cdx.json" \
        "$WORKDIR/sbom.cdx.json"
    docker cp -q "$container:/usr/share/sbom/collabora-online-image-sbom.spdx.json" \
        "$WORKDIR/sbom.spdx.json" || true
    docker rm -f "$container" >/dev/null
}

# Key-based signing without a transparency log, on the classic sidecar-tag
# layout (sha256-<digest>.sig / .att) that ZenDiS uses and that registries
# without the OCI referrers API can serve. cosign >= 3 defaults to the new
# sigstore bundle format and an implicit signing config, so both have to be
# switched off there; cosign 2.x has neither flag and behaves this way anyway.
COSIGN_FLAGS="--yes --tlog-upload=false"
if cosign sign --help 2>&1 | grep -q use-signing-config; then
    COSIGN_FLAGS="$COSIGN_FLAGS --use-signing-config=false --new-bundle-format=false"
fi

attest() {
    pinned="$1"; type="$2"; predicate="$3"
    echo "  attest $type"
    cosign attest $COSIGN_FLAGS --key "$KEY" --type "$type" \
        --predicate "$predicate" "$pinned"
}

publish_digest() {
    repository="$1"; digest="$2"
    pinned="$repository@$digest"
    echo "== $pinned"

    echo "  sign"
    cosign sign $COSIGN_FLAGS --key "$KEY" "$pinned"

    extract_sboms "$pinned"
    attest "$pinned" cyclonedx "$WORKDIR/sbom.cdx.json"
    if [ -s "$WORKDIR/sbom.spdx.json" ]; then
        attest "$pinned" spdxjson "$WORKDIR/sbom.spdx.json"
    fi

    if command -v trivy >/dev/null; then
        echo "  scan (trivy)"
        # scan the SBOM, not the filesystem: the Collabora-published packages
        # are only identifiable through the components the SBOM carries
        trivy sbom --format sarif --output "$WORKDIR/scan.sarif" \
            "$WORKDIR/sbom.cdx.json"
        attest "$pinned" \
            "https://www.schemastore.org/schemas/json/sarif-2.1.0.json" \
            "$WORKDIR/scan.sarif"
        trivy sbom --format json --output "$WORKDIR/scan.json" \
            "$WORKDIR/sbom.cdx.json"
        python3 "$SCRIPT_DIR/generate-vex.py" --vex "$VEX_FILE" \
            --scan "$WORKDIR/scan.json"
    else
        echo "  trivy not found: skipping the scan attestation and the VEX" \
             "refresh (the checked-in VEX is still attested)" >&2
    fi
    attest "$pinned" "https://cyclonedx.org/vex" "$VEX_FILE"

    attest "$pinned" slsaprovenance1 "$(make_provenance "$pinned")"
}

# Every per-platform manifest digest of a ref (or the ref's own digest).
ref_digests() {
    docker buildx imagetools inspect --format '{{json .}}' "$1" | python3 -c '
import json, sys
data = json.load(sys.stdin)
manifest = data.get("manifest", {})
entries = [m["digest"] for m in manifest.get("manifests", [])
           if m.get("platform", {}).get("os") not in (None, "unknown")]
print("\n".join(entries if entries else [manifest.get("digest", "")]))
'
}

for ref in "$@"; do
    if [ -n "$PUSH" ]; then
        echo "== push $ref"
        docker push -q "$ref"
    fi
done

if [ -n "$MANIFEST_TAG" ]; then
    echo "== manifest list $MANIFEST_TAG"
    docker buildx imagetools create -t "$MANIFEST_TAG" "$@"
fi

for ref in "$@"; do
    repository=${ref%%@*}; repository=${repository%:*}
    for digest in $(ref_digests "$ref"); do
        [ -n "$digest" ] || continue
        publish_digest "$repository" "$digest"
    done
done

echo "done. verify with:"
echo "  cosign verify --key docker/cosign.pub <ref>"
echo "  cosign verify-attestation --key docker/cosign.pub --type cyclonedx <ref>"
