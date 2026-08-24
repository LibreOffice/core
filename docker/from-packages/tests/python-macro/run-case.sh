#!/bin/bash
# Run one Python-macro test case against a Collabora Online container image.
#
# Usage: run-case.sh <image> <label> [--install-pyuno]
#
# Starts the image with macro execution enabled and the test macro bind-mounted
# into /opt/collaboraoffice/share/Scripts/python (the 'share' script location
# the CallPythonScript postMessage API uses), then drives it over the websocket
# protocol and checks whether the macro actually runs.
#
# --install-pyuno additionally tries the classic enablement path used on the
# Debian-based image: docker exec + apt-get install of the pyuno packages.
set -u

IMAGE=$1
LABEL=$2
INSTALL=${3:-}

DIR=$(cd "$(dirname "$0")" && pwd)
NAME=cool-pymacro-test
COOLPORT=9980
WOPIPORT=9981
GW=$(docker network inspect bridge --format '{{(index .IPAM.Config 0).Gateway}}')
DOC=$DIR/test.odt

cleanup() {
    docker rm -f $NAME >/dev/null 2>&1
    [ -n "${WOPI_PID:-}" ] && kill "$WOPI_PID" 2>/dev/null
}
trap cleanup EXIT

echo "===================================================================="
echo "CASE: $LABEL"
echo "IMAGE: $IMAGE"
echo "===================================================================="

python3 "$DIR/wopi.py" $WOPIPORT "$DOC" 2>"$DIR/wopi-$LABEL.log" &
WOPI_PID=$!

# The pyuno package installs its own files under share/Scripts/python, so in
# --install-pyuno mode the macro is copied in after the install instead of
# bind-mounting over the package's directory.
MOUNT_ARGS=(-v "$DIR/macros:/opt/collaboraoffice/share/Scripts/python:ro")
[ "$INSTALL" = "--install-pyuno" ] && MOUNT_ARGS=()

# The config overrides are passed both via extra_params and as container
# arguments, so the test works across image generations: the Debian-based
# image's start script reads extra_params and forwards "$@", the distroless
# entrypoint appends arguments and honours extra_params via --use-env-vars.
# Duplicate --o overrides are harmless.
OVERRIDES=(--o:ssl.enable=false
           --o:security.enable_macros_execution=true
           --o:security.macro_security_level=0)

docker rm -f $NAME >/dev/null 2>&1
docker run -d --name $NAME -p 127.0.0.1:$COOLPORT:9980 \
    -e "aliasgroup1=http://${GW}:${WOPIPORT}" \
    -e "extra_params=${OVERRIDES[*]}" \
    "${MOUNT_ARGS[@]}" \
    "$IMAGE" "${OVERRIDES[@]}" >/dev/null \
    || { echo "RESULT($LABEL): ERROR - container failed to start"; exit 2; }

wait_ready() {
    for _ in $(seq 60); do
        curl -sf "http://127.0.0.1:$COOLPORT/" >/dev/null 2>&1 && return 0
        sleep 2
    done
    return 1
}

if ! wait_ready; then
    echo "RESULT($LABEL): ERROR - coolwsd did not become ready"
    docker logs $NAME 2>&1 | tail -20
    exit 2
fi

if [ "$INSTALL" = "--install-pyuno" ]; then
    echo "--- attempting the classic enablement path: docker exec + apt-get"
    V=$(docker exec -u root $NAME dpkg-query -W -f '${Version}' collaboraofficebasis-core 2>/dev/null)
    echo "    installed collaboraofficebasis-core version: ${V:-<unknown>}"
    if ! docker exec -u root $NAME sh -c "apt-get update -qq && \
            apt-get install -y -qq collaboraofficebasis-pyuno=$V collaboraofficebasis-python-script-provider=$V" \
            > "$DIR/install-$LABEL.log" 2>&1; then
        echo "RESULT($LABEL): ENABLEMENT-PATH-FAILED (docker exec / apt-get did not work)"
        tail -5 "$DIR/install-$LABEL.log" | sed 's/^/    /'
        exit 3
    fi
    echo "    pyuno packages installed; adding test macro and restarting container"
    docker cp "$DIR/macros/macro_test.py" \
        "$NAME:/opt/collaboraoffice/share/Scripts/python/macro_test.py"
    docker restart $NAME >/dev/null
    if ! wait_ready; then
        echo "RESULT($LABEL): ERROR - coolwsd did not come back after install"
        exit 2
    fi
fi

cd "$DIR"
python3 "$DIR/wsclient.py" "ws://127.0.0.1:$COOLPORT" "http://${GW}:${WOPIPORT}/wopi/files/test.odt" 60
RC=$?
mv -f wsclient.log "wsclient-$LABEL.log" 2>/dev/null

echo "--- relevant kit/wsd log lines:"
docker logs $NAME 2>&1 | grep -ioE ".{0,120}(pyuno|pythonscript|scriptprovider|vnd\.sun\.star\.script|macro_test)[^ ]*.{0,80}" | tail -10 | sed 's/^/    /'

case $RC in
    0) echo "RESULT($LABEL): PASS - Python macro executed" ;;
    1) echo "RESULT($LABEL): FAIL - Python macro did not execute" ;;
    *) echo "RESULT($LABEL): ERROR - document load / protocol problem (see wsclient-$LABEL.log)" ;;
esac
exit $RC
