#! /bin/bash
set -e
trap 'echo SIGHUP >>/tmp/sberg' SIGHUP
trap 'echo SIGINT >>/tmp/sberg' SIGINT
trap 'echo SIGQUIT >>/tmp/sberg' SIGQUIT
trap 'echo SIGKILL >>/tmp/sberg' SIGKILL
trap 'echo SIGTERM >>/tmp/sberg' SIGTERM
echo START >>/tmp/sberg
while true; do sleep 1; done
