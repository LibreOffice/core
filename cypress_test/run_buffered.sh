#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ -z "${NODE_PATH}" ]; then
    BUILDDIR=${DIR}
else
    BUILDDIR=$(dirname ${NODE_PATH})
fi

CYPRESS_BINARY="${BUILDDIR}/node_modules/cypress/bin/cypress"
DESKTOP_TEST_FOLDER="${DIR}/integration_tests/desktop/"
MOBILE_TEST_FOLDER="${DIR}/integration_tests/mobile/"
MULTIUSER_TEST_FOLDER="${DIR}/integration_tests/multiuser/"
ERROR_LOG="${BUILDDIR}/workdir/error.log"

print_help ()
{
    echo "Usage: run_buffered.sh --spec <name_spec.js> OPTIONS"
    echo "Runs a specified cypress test with output buffered to a log file"
    echo ""
    echo "   --spec <file>              The test file we need to run"
    echo "   --log-file <file>          Log output to this test"
    echo "   --config <string>          Configure options passed to cypress"
    echo "   --env <string>             Cypress own environment variables"
    echo "   --type <string>            Type of the test (e.g. mobile, desktop)"
    echo "   --browser <file>           Path to the browser binary"
    echo "   --second-chance            Enable second chance"
    exit 1
}

TEST_FILE=
TEST_LOG=
TEST_CONFIG=
TEST_CONFIG_FILE=
TEST_ENV=
TEST_TYPE=
BROWSER=
SECOND_CHANCE=false
while test $# -gt 0; do
  case $1 in
      --spec)             TEST_FILE=$2; shift;;
      --log-file)         TEST_LOG=$2; shift;;
      --config-file)      TEST_CONFIG_FILE=$2; shift;;
      --config)           TEST_CONFIG=$2; shift;;
      --env)              TEST_ENV=$2; shift;;
      --type)             TEST_TYPE=$2; shift;;
      --browser)          BROWSER=$2; shift;;
      --help)             print_help ;;
  -*) ;; # ignore
  esac
  shift
done
TEST_ERROR="${TEST_LOG}.error"

TEST_FILE_PATH=
if [ "${TEST_TYPE}" = "desktop" -o "${TEST_TYPE}" = "interfer-desktop" ]; then
    TEST_FILE_PATH=${DESKTOP_TEST_FOLDER}${TEST_FILE};
elif [ "${TEST_TYPE}" = "mobile" -o "${TEST_TYPE}" = "interfer-mobile" ]; then
    TEST_FILE_PATH=${MOBILE_TEST_FOLDER}${TEST_FILE};
elif [ "${TEST_TYPE}" = "multi-user" ]; then
    TEST_FILE_PATH=${MULTIUSER_TEST_FOLDER}${TEST_FILE};
elif [ "${TEST_TYPE}" = "interfer" ]; then
    TEST_FILE_PATH="${DIR}/integration_tests/common/"${TEST_FILE};
fi

RUN_COMMAND="setsid ${CYPRESS_BINARY} run \
    --browser ${BROWSER} \
    --headless \
    --config-file ${TEST_CONFIG_FILE}\
    --config ${TEST_CONFIG}\
    --env ${TEST_ENV}\
    --spec=${TEST_FILE_PATH}"

print_error() {
    SPEC=${TEST_FILE}
    COMMAND=${TEST_TYPE}
    if [ "${TEST_TYPE}" = "interfer" ]; then
        echo -e "\n\
        CypressError: the interference user failed.\n\n\
        For running this test again, you need to find the related test user.\n" >> ${ERROR_LOG}
        return
    fi

    if [ "${TEST_TYPE}" = "multi-user" ]; then
        COMMAND="multi"
        SPEC=${SPEC%"_user1_spec.js"}
        SPEC=${SPEC%"_user2_spec.js"}
    fi

    if [ "${USER_INTERFACE}" == "notebookbar" ] && [ "${TEST_TYPE}" == "desktop" ]; then
        echo -e "\n\
        CypressError: a test failed, please do one of the following:\n\n\
        Run the failing test in headless mode:\n\
        \tmake -C cypress_test USER_INTERFACE=notebookbar check-${COMMAND} spec=${SPEC}\n" >> ${ERROR_LOG}
    else
        echo -e "\n\
        CypressError: a test failed, please do one of the following:\n\n\
        Run the failing test in headless mode:\n\
        \tmake -C cypress_test check-${COMMAND} spec=${SPEC}\n" >> ${ERROR_LOG}
    fi

    if [ "${TEST_TYPE}" == "mobile" -o "${TEST_TYPE}" == "desktop" ]; then
        if [ "${USER_INTERFACE}" == "notebookbar" ]; then
            echo -e "\
        Run the failing test with video recording:\n\
            \tmake -C cypress_test ENABLE_VIDEO_REC="1" USER_INTERFACE=notebookbar check-${COMMAND} spec=${SPEC}\n" >> ${ERROR_LOG}
        else
            echo -e "\
            Run the failing test with video recording:\n\
            \tmake -C cypress_test ENABLE_VIDEO_REC="1" check-${COMMAND} spec=${SPEC}\n" >> ${ERROR_LOG}
        fi
    fi

    if [ "${TEST_TYPE}" != "multi-user" ]; then
    if [ "${USER_INTERFACE}" == "notebookbar" ]; then
        echo -e "\
        Open the failing test in the interactive test runner:\n\
        \tmake -C cypress_test USER_INTERFACE=notebookbar run-${COMMAND} spec=${SPEC}\n" >> ${ERROR_LOG}
    else
        echo -e "\
        Open the failing test in the interactive test runner:\n\
        \tmake -C cypress_test run-${COMMAND} spec=${SPEC}\n" >> ${ERROR_LOG}
    fi
    elif [[ ${TEST_FILE} == *"user1"* ]]; then
    echo -e "\
    Open the failing test in the interactive test runner:\n\
    \tmake -C cypress_test run-${COMMAND} spec=${SPEC} user=1\n" >> ${ERROR_LOG}
    else
    echo -e "\
    Open the failing test in the interactive test runner:\n\
    \tmake -C cypress_test run-${COMMAND} spec=${SPEC} user=2\n" >> ${ERROR_LOG}
    fi
}

mkdir -p `dirname ${TEST_LOG}`
touch ${TEST_LOG}
rm -rf ${TEST_ERROR}
echo "`echo ${RUN_COMMAND} && ${RUN_COMMAND} || touch ${TEST_ERROR}`" > ${TEST_LOG} 2>&1
if [ ! -f ${TEST_ERROR} ];
    then cat ${TEST_LOG};
    else echo -e "Cypress test failed: ${TEST_FILE}\n" && \
        cat ${TEST_LOG} >> ${ERROR_LOG} && \
        print_error;
fi;

# vim:set shiftwidth=4 expandtab:
