# A LibreTranslate server, the open source machine translation API, for
# Collabora Online's Translate command. LibreTranslate itself is not in this
# repository: its release is installed from PyPI while the image builds,
# pinned by hash in requirements.txt, and the translation models of the
# chosen languages are fetched from the Argos Translate package index into
# the image, so the container needs no network at start. LibreTranslate is
# released under the AGPL 3; the licence text is in the installed package.
#
#   docker build -t libretranslate docker/libretranslate
#   docker run --rm -p 5000:5000 libretranslate
#   curl -H 'Content-Type: application/json' \
#        -d '{"q":"The house is green.","source":"en","target":"de","format":"text"}' \
#        http://127.0.0.1:5000/translate
#
# The server has no authentication by default and no TLS: keep it on an
# internal network, as compose.yaml here does.

# The languages to translate between, two at least. Every model goes to or
# from English, so a translation between two other languages goes through
# English. Each direction is a package of some 100 MB; the image grows with
# the list.
ARG LIBRETRANSLATE_LANGUAGES=en,de,fr,es,it,nl,pt

FROM python:3.12-slim AS build
ARG LIBRETRANSLATE_LANGUAGES
# Argos Translate keeps its models under the data directory: the language
# packages, and the sentence boundary models it fetches with them. All of
# it goes into the image, so the container needs no network at start.
ENV XDG_DATA_HOME=/srv/argos
COPY requirements.txt /tmp/requirements.txt
RUN set -e; \
    python3 -m venv /srv/libretranslate; \
    /srv/libretranslate/bin/pip install --no-cache-dir --require-hashes -r /tmp/requirements.txt
# The models of the chosen languages, from the Argos Translate package index.
RUN /srv/libretranslate/bin/python3 -c "import libretranslate.init as lt; \
    lt.check_and_install_models(force=True, load_only_lang_codes='${LIBRETRANSLATE_LANGUAGES}'.split(','))"

FROM python:3.12-slim
LABEL org.opencontainers.image.title="LibreTranslate server" \
      org.opencontainers.image.description="LibreTranslate, the open source machine translation API, as a service for Collabora Online" \
      org.opencontainers.image.licenses="AGPL-3.0-only"
ARG LIBRETRANSLATE_LANGUAGES
# Every setting of the server is a command line argument or the LT_
# environment variable of its name; see docs.libretranslate.com.
ENV XDG_DATA_HOME=/srv/argos \
    LT_HOST=0.0.0.0 \
    LT_PORT=5000 \
    LT_LOAD_ONLY=${LIBRETRANSLATE_LANGUAGES} \
    LT_DISABLE_WEB_UI=true \
    LT_DISABLE_FILES_TRANSLATION=true \
    LT_THREADS=4 \
    PYTHONUNBUFFERED=1
# Argos Translate keeps a cache and a config directory under the home
# directory, and makes them at start: the user needs one. The key database,
# when keys are required, is the one thing the server writes next to
# itself: that directory is the user's.
RUN useradd --system --uid 1001 --create-home --home-dir /var/lib/libretranslate libretranslate \
    && install -d -o libretranslate -g libretranslate /srv/libretranslate/db
COPY --from=build --chown=root:root /srv/libretranslate /srv/libretranslate
COPY --from=build --chown=root:root /srv/argos /srv/argos
WORKDIR /srv/libretranslate
USER libretranslate
EXPOSE 5000
HEALTHCHECK --interval=30s --timeout=5s --start-period=60s \
    CMD /srv/libretranslate/bin/python3 -c 'import urllib.request; urllib.request.urlopen("http://127.0.0.1:5000/languages", timeout=4)' || exit 1
CMD ["/srv/libretranslate/bin/libretranslate"]
