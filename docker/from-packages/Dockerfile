# syntax=docker/dockerfile:1.4
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Base image for the final, hardened runtime stage. ZenDiS / openCode provides
# minimal distroless images that contain only glibc, openssl, ca-certificates
# and timezone data. The '-minimal' variant has no shell and no package
# manager; drop the '-minimal' suffix for the standard variant, which ships a
# static busybox useful for debugging. Bump this as new releases appear, see
# https://gitlab.opencode.de/oci-community/images/zendis/base
ARG runtime_base="registry.opencode.de/oci-community/images/zendis/base:1.0.0-rc.6-minimal"

# Metadata, applied to the final image (see the runtime stage below).
ARG author
ARG releasenotes
ARG version
ARG onlinetag

# Named alias of the runtime base, so the builder can source the jail's
# glibc loader / NSS / resolver and the CA trust store from it (see below).
FROM ${runtime_base} AS hardened

# ---------------------------------------------------------------------------
# Builder stage: install Collabora Online from packages on Debian exactly as
# before, then assemble a self-contained root filesystem to hand to the base.
# ---------------------------------------------------------------------------
FROM debian:stable-slim AS builder

# repo: can be 'repos', 'repos-staging', 'repos-snapshot'
# Optional. The default is 'repos'.
ARG repo

# type: can be
# 'code' - Collabora Online Development Edition
# 'cool' - Collabora Online, to build this you need to give your secret URL part from https://support.collaboraoffice.com, i.e. you have to be Collabora partner or customer
# 'key'  - Collabora Online, the license key enabled version.
# Optional. The default is 'code'.
ARG type

# Optional. If defined then brand package will not be installed.
ARG nobrand

# UTF-8 locale is needed to handle file names with non-ASCII characters
ENV LC_CTYPE C.UTF-8

# Collabora apt signing keys (release for stable repos, snapshot for repos-snapshot)
ADD --chmod=644 https://www.collaboraoffice.com/downloads/gpg/collaboraonline-release-keyring.gpg /usr/share/keyrings/
ADD --chmod=644 https://www.collaboraoffice.com/downloads/gpg/collaboraonline-snapshot-keyring.gpg /usr/share/keyrings/

# Mount secret URL part (for type 'cool') securely
RUN --mount=type=secret,id=secret_key \
# Capture the pristine package set so we can later tell exactly what the
# Collabora install added (Collabora Online plus its full dependency closure)
    dpkg-query -W -f '${Package}\n' | sort > /tmp/pkgs.before && \
# Update repos before installing packages
    apt-get update && \
# Install dependencies of installer of Collabora Online (setcap is build-time only)
    apt-get -y install libcap2-bin ca-certificates && \
# Setup Collabora repo
    repourl="https://collaboraoffice.com/${repo:-repos}/CollaboraOnline/"; \
    secret_key=$(cat /run/secrets/secret_key); \
    if [ "$type" = "cool" ] && [ -n ${secret_key+set} ]; then \
        echo "Based on the provided build arguments Collabora Online from customer repo will be used."; \
        repourl="${repourl}26.04/customer-deb-${secret_key}/"; \
    elif [ "$type" = "key" ]; then \
        echo "Based on the provided build arguments license key enabled Collabora Online will be used."; \
        repourl="${repourl}26.04-key/"; \
    else \
        echo "Based on the provided build arguments Collabora Online Development Edition will be used."; \
        repourl="${repourl}26.04-CODE/CODE-deb/"; \
    fi && \
    keyring=collaboraonline-release-keyring; \
    if [ "$repo" = "repos-snapshot" ]; then \
        keyring=collaboraonline-snapshot-keyring; \
    fi && \
    echo "deb [signed-by=/usr/share/keyrings/${keyring}.gpg] ${repourl} /" > /etc/apt/sources.list.d/collabora.list && \
# Update repos again
    apt-get update && \
# Install Collabora Online (Collabora Office is a dependency)
    apt-get -y install coolwsd coolwsd-deprecated \
# Install all dictionaries
                       collaboraoffice-dict-* \
# Install all language packs
                       collaboraofficebasis-ar \
                       collaboraofficebasis-bg \
                       collaboraofficebasis-ca \
                       collaboraofficebasis-cs \
                       collaboraofficebasis-da \
                       collaboraofficebasis-de \
                       collaboraofficebasis-el \
                       collaboraofficebasis-en-gb \
                       collaboraofficebasis-en-us \
                       collaboraofficebasis-eo \
                       collaboraofficebasis-es \
                       collaboraofficebasis-eu \
                       collaboraofficebasis-fi \
                       collaboraofficebasis-fr \
                       collaboraofficebasis-gl \
                       collaboraofficebasis-he \
                       collaboraofficebasis-hr \
                       collaboraofficebasis-hu \
                       collaboraofficebasis-id \
                       collaboraofficebasis-is \
                       collaboraofficebasis-it \
                       collaboraofficebasis-ja \
                       collaboraofficebasis-ko \
                       collaboraofficebasis-lo \
                       collaboraofficebasis-nb \
                       collaboraofficebasis-nl \
                       collaboraofficebasis-oc \
                       collaboraofficebasis-pl \
                       collaboraofficebasis-pt \
                       collaboraofficebasis-pt-br \
                       collaboraofficebasis-ru \
                       collaboraofficebasis-sk \
                       collaboraofficebasis-sl \
                       collaboraofficebasis-sq \
                       collaboraofficebasis-sv \
                       collaboraofficebasis-tr \
                       collaboraofficebasis-uk \
                       collaboraofficebasis-vi \
                       collaboraofficebasis-zh-cn \
                       collaboraofficebasis-zh-tw && \
# Install brand package (or not)
    if [ -z "$nobrand" ]; then \
        if [ "$type" = "cool" ] || [ "$type" = "key" ]; then \
            apt-get -y install collabora-online-brand; \
        else \
            apt-get -y install code-brand; \
        fi; \
    fi && \
# Change cool uid/gid to 1001/1001
    find /opt/cool -user cool -exec chown -h 1001:1001 {} \; && \
    usermod -u 1001 cool && groupmod -g 1001 cool && \
    chown cool:cool /etc/coolwsd/coolwsd.xml && \
# Change permission so that other users and groups can read the config file
    chmod 644 /etc/coolwsd/coolwsd.xml && \
# Fix ownership of config directory that will be modified on start of the container by cool user
    chown cool:cool /etc/coolwsd && \
# Allow the root group to append to /etc/passwd so coolwsd can map an arbitrary
# (e.g. OpenShift-assigned) UID to the 'cool' user at startup
    chmod g+w /etc/passwd

# Assemble the root filesystem to hand to the hardened base image. The base
# image's own files are made available under /hardened so the assembler can
# source the jail's glibc loader / NSS / resolver and the CA trust store from
# the target rather than from the Debian builder.
COPY --from=hardened / /hardened/
COPY /scripts/assemble-rootfs.sh /usr/local/bin/assemble-rootfs.sh
RUN HARDENED_ROOT=/hardened /usr/local/bin/assemble-rootfs.sh

# ---------------------------------------------------------------------------
# Capability check: the runtime stage relies on COPY --from carrying the jail's
# file capabilities (the security.capability xattr on coolforkit-caps and
# coolmount). That holds for modern BuildKit but not for the legacy builder and
# not necessarily for every rootless/older setup, so prove it here and fail the
# build rather than ship a jail that cannot start. Copying the two binaries out
# of the builder exercises the exact same COPY --from mechanism the runtime
# stage uses; getcap is already present here (libcap2-bin in the builder).
# ---------------------------------------------------------------------------
FROM builder AS verify-caps
COPY --from=builder /rootfs/usr/bin/coolforkit-caps /rootfs/usr/bin/coolmount /probe/
RUN set -eu; \
    fc="$(/sbin/getcap /probe/coolforkit-caps)"; \
    mc="$(/sbin/getcap /probe/coolmount)"; \
    echo "post-COPY caps -- coolforkit-caps: ${fc:-<none>} | coolmount: ${mc:-<none>}"; \
    case "$fc" in *cap_sys_chroot*) ;; *) echo "FATAL: cap_sys_chroot lost on COPY --from; BuildKit is not preserving the security.capability xattr" >&2; exit 1;; esac; \
    case "$mc" in *cap_sys_admin*) ;; *) echo "FATAL: cap_sys_admin lost on COPY --from; BuildKit is not preserving the security.capability xattr" >&2; exit 1;; esac; \
    touch /caps-verified

# ---------------------------------------------------------------------------
# Runtime stage: the hardened, distroless base with only Collabora Online and
# its runtime dependencies copied in. No shell, no package manager.
# ---------------------------------------------------------------------------
FROM ${runtime_base}

# some descriptive metadata - docker image inspect
ARG author
LABEL author=${author:-"Collabora Productivity Ltd."}
LABEL description="Collabora Online is a powerful collaborative Office suite that supports all major document, spreadsheet and presentation file formats, which you can integrate into your own infrastructure. Collabora Online provides data security and sovereignty, and is ideally suited to the demands of a modern distributed working environment. Delivering a familiar look and feel, Collabora Online represents a real alternative to other big-brands solutions, giving you control and flexibility."
ARG releasenotes
LABEL release.notes=${releasenotes:-"https://www.collaboraonline.com/code-26-04-release-notes/"}
ARG version
LABEL version=${version:-"26.04.0.1"}
ARG onlinetag
LABEL commit.history.core="https://gerrit.collaboraoffice.com/plugins/gitiles/online/+log/${onlinetag}"

# UTF-8 locale is needed to handle file names with non-ASCII characters
ENV LC_CTYPE C.UTF-8

# Bring in Collabora Online and all its runtime dependencies, assembled in the
# builder. This relies on COPY --from carrying the file capabilities on
# coolforkit-caps and coolmount (security.capability xattr); the verify-caps
# stage proves that holds in this build environment.
COPY --from=builder /rootfs /

# Force the capability check to run as part of every build: this pulls in the
# verify-caps stage, which fails the build if the capabilities did not survive.
COPY --from=verify-caps /caps-verified /opt/cool/.caps-verified

# coolwsd listens on port 9980
EXPOSE 9980

# Switch to cool user (use numeric user id to be compatible with Kubernetes Pod Security Policies)
USER 1001

# Start coolwsd directly, with no shell in between: self-signed certificate
# generation (unless DONT_GEN_SSL_CERT is set) and arbitrary-UID handling are
# done inside coolwsd itself. The configuration is still driven dynamically by
# environment variables (--use-env-vars).
ENTRYPOINT ["/usr/bin/coolwsd", \
            "--use-env-vars", \
            "--o:sys_template_path=/opt/cool/systemplate", \
            "--o:child_root_path=/opt/cool/child-roots", \
            "--o:file_server_root_path=/usr/share/coolwsd", \
            "--o:cache_files.path=/opt/cool/cache", \
            "--o:logging.color=false", \
            "--o:stop_on_config_change=true"]
