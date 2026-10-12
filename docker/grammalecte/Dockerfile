# A Grammalecte server, the French grammar checker, for Collabora Online's
# remote sentence checker. Grammalecte itself is not in this repository: its
# release archive is fetched while the image builds, and its licence (GPL 3)
# is in the archive.
#
#   docker build -t grammalecte .
#   docker run --rm -p 8080:8080 grammalecte
#   curl -d 'text=Il%20sont%20parti.' http://127.0.0.1:8080/gc_text/fr
#
# The server has no authentication and no TLS: keep it on an internal
# network, as compose.yaml here does.

ARG GRAMMALECTE_VERSION=2.3.0
ARG GRAMMALECTE_SHA256=aaa4219704857778038ecc1db18f6a907994c0125b8e762dcecc69130280684f

FROM python:3-slim AS fetch
ARG GRAMMALECTE_VERSION
ARG GRAMMALECTE_SHA256
WORKDIR /src
ADD https://grammalecte.net/zip/Grammalecte-fr-v${GRAMMALECTE_VERSION}.zip grammalecte.zip
RUN set -e; \
    echo "${GRAMMALECTE_SHA256}  grammalecte.zip" | sha256sum -c -; \
    python3 -c 'import zipfile; zipfile.ZipFile("grammalecte.zip").extractall("/srv/grammalecte")'

FROM python:3-slim
LABEL org.opencontainers.image.title="Grammalecte server" \
      org.opencontainers.image.description="Grammalecte, the French grammar checker, as an HTTP service for Collabora Online" \
      org.opencontainers.image.licenses="GPL-3.0-only"
ARG GRAMMALECTE_VERSION
ENV GRAMMALECTE_VERSION=${GRAMMALECTE_VERSION} \
    PYTHONUNBUFFERED=1
RUN useradd --system --uid 1001 --no-create-home grammalecte
COPY --from=fetch --chown=root:root /srv/grammalecte /srv/grammalecte
WORKDIR /srv/grammalecte
USER grammalecte
EXPOSE 8080
HEALTHCHECK --interval=30s --timeout=5s --start-period=20s \
    CMD python3 -c 'import urllib.request; urllib.request.urlopen("http://127.0.0.1:8080/get_options/fr", timeout=4)' || exit 1
# -mp: how many checker processes answer requests; raise it for a busy server.
CMD ["python3", "grammalecte-server.py", "-ht", "0.0.0.0", "-p", "8080", "-mp", "2"]
