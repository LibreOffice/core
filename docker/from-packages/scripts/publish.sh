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
#   publish.sh [-k cosign-key-ref] [-m manifest-list-tag] [-n] [-T] image-ref...
#
#   -k  cosign key reference: a file path, env://COSIGN_KEY, or a KMS URI
#       (default: env://COSIGN_KEY; COSIGN_PASSWORD is honoured by cosign).
#       NOTE: docker/from-packages/secret_key is the apt repository secret,
#       NOT a signing key - never pass it here. The private counterpart of
#       docker/cosign.pub is kept outside this repository.
#   -m  additionally assemble the given refs into a multi-arch manifest list
#       under this tag (docker buildx imagetools create) and push it
#   -n  no-push: the refs are already pushed, only sign and attest
#   -T  do not record the signatures in the public transparency log. By
#       default they are recorded, like the signatures the publishing jobs
#       make themselves, so that 'cosign verify-attestation' works without
#       further flags; use this for a private registry, an offline run or a
#       rehearsal, and verify with --insecure-ignore-tlog=true.
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
NO_TLOG=
while getopts k:m:nTh opt; do
    case "$opt" in
        k) KEY="$OPTARG" ;;
        m) MANIFEST_TAG="$OPTARG" ;;
        n) PUSH= ;;
        T) NO_TLOG=yes ;;
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
    # no 'docker cp -q': that flag only exists from Docker 25, and the
    # builders run older versions
    docker cp "$container:/usr/share/sbom/collabora-online.cdx.json" \
        "$WORKDIR/sbom.cdx.json"
    docker cp "$container:/usr/share/sbom/collabora-online-image-sbom.spdx.json" \
        "$WORKDIR/sbom.spdx.json" 2>/dev/null || true
    docker rm -f "$container" >/dev/null
}

# Key-based signing without a transparency log, on the classic sidecar-tag
# layout (sha256-<digest>.sig / .att) that ZenDiS uses and that registries
# without the OCI referrers API can serve. cosign >= 3 defaults to the new
# sigstore bundle format and an implicit signing config, so both have to be
# switched off there; cosign 2.x has neither flag and behaves this way anyway.
COSIGN_FLAGS="--yes"
if [ -n "$NO_TLOG" ]; then
    COSIGN_FLAGS="$COSIGN_FLAGS --tlog-upload=false"
fi
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
        # grype additionally matches the CPEs of the statically linked C
        # libraries, which trivy does not look at; without it the VEX is
        # refreshed from half the findings
        if command -v grype >/dev/null; then
            echo "  scan (grype, for the VEX)"
            grype -q "sbom:$WORKDIR/sbom.cdx.json" -o json \
                > "$WORKDIR/scan-grype.json"
            python3 "$SCRIPT_DIR/generate-vex.py" --vex "$VEX_FILE" \
                --scan "$WORKDIR/scan-grype.json"
        else
            echo "  grype not found: the VEX refresh will miss CPE-only" \
                 "findings (the statically linked C libraries)" >&2
        fi
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
# Digest of each platform manifest behind a reference (or of the reference
# itself when it is not an index). Several sources are tried because the
# shape of what they print differs between versions, and buildx is a plugin
# the builder may not have; publish_ref below fails if none of them answers,
# rather than silently signing nothing.
digests_from_json() {
    python3 -c '
import json, sys
data = json.load(sys.stdin)
if isinstance(data, dict) and "manifest" in data:   # buildx imagetools
    data = data["manifest"]
if isinstance(data, list):                          # docker manifest -v, index
    data = {"manifests": [{"digest": e["Descriptor"]["digest"],
                           "platform": e["Descriptor"].get("platform", {})}
                          for e in data]}
elif "Descriptor" in data:                          # docker manifest -v, single
    data = {"digest": data["Descriptor"]["digest"]}
entries = [m["digest"] for m in data.get("manifests", [])
           if m.get("platform", {}).get("os") not in (None, "unknown")]
if not entries and data.get("digest"):
    entries = [data["digest"]]
print("\n".join(entries))
' 2>/dev/null
}

ref_digests() {
    docker buildx imagetools inspect --format '{{json .}}' "$1" 2>/dev/null \
        | digests_from_json && return 0
    docker buildx imagetools inspect --format '{{json .Manifest}}' "$1" \
        2>/dev/null | digests_from_json && return 0
    docker manifest inspect -v "$1" 2>/dev/null | digests_from_json
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
    found=
    for digest in $(ref_digests "$ref"); do
        [ -n "$digest" ] || continue
        found=yes
        publish_digest "$repository" "$digest"
    done
    if [ -z "$found" ]; then
        echo "publish.sh: cannot resolve a digest for $ref - is it pushed?" \
             "Nothing was signed or attested." >&2
        exit 1
    fi
done

# Signatures and attestations are attached to the per-platform digests, which
# is what ZenDiS does too and the only thing that makes sense: the SBOM of an
# arm64 image is not the SBOM of the amd64 one. A multi-arch tag points at the
# index, whose digest carries neither, so verification names a platform.
echo "done. verify a per-platform reference, not a multi-arch tag:"
for ref in "$@"; do
    echo "  cosign verify --key docker/cosign.pub${NO_TLOG:+ --insecure-ignore-tlog=true} $ref"
    echo "  cosign verify-attestation --key docker/cosign.pub${NO_TLOG:+ --insecure-ignore-tlog=true} --type cyclonedx $ref"
    break
done
