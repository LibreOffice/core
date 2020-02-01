#!/bin/bash -e

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if [[ "$#" != "1" ]] ; then
  cat <<EOF
Usage: run "$0 COMMAND" from the top of your workspace,
where COMMAND is one of

  install
  deploy

Set VERBOSE in the environment to get more information.

EOF

  exit 1
fi

set -o errexit
set -o nounset

case "$1" in
install)
    command="api_install"
    ;;
deploy)
    command="api_deploy"
    ;;
*)
    echo "unknown command $1"
    exit 1
    ;;
esac

if [[ "${VERBOSE:-x}" != "x" ]]; then
  set -o xtrace
fi

if [[ `which bazelisk` ]]; then
  BAZEL_CMD=bazelisk
else
  BAZEL_CMD=bazel
fi

${BAZEL_CMD} build //solenv/maven:gen_${command} || \
  { echo "${BAZEL_CMD} failed to build gen_${command}. Use VERBOSE=1 for more info" ; exit 1 ; }

export OUTPUT_BASE=`bazel info output_base`
./bazel-bin/solenv/maven/${command}.sh
