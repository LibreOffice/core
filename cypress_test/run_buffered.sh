#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ -z "${NODE_PATH}" ]; then
    BUILDDIR=${DIR}
else
    BUILDDIR=$(dirname ${NODE_PATH})
fi

CYPRESS_BINARY="${BUILDDIR}/node_modules/cypress/bin/cypress"
DESKTOP_TEST_FOLDER="${DIR}/integration_tests/desktop/"
IDLE_TEST_FOLDER="${DIR}/integration_tests/idle/"
MOBILE_TEST_FOLDER="${DIR}/integration_tests/mobile/"
MULTIUSER_TEST_FOLDER="${DIR}/integration_tests/multiuser/"
LIGHTHOUSE_TEST_FOLDER="${DIR}/integration_tests/lighthouse/"
ERROR_LOG="${BUILDDIR}/workdir/error.log"
SPEC_TIMINGS="${BUILDDIR}/workdir/spec-timings.txt"

print_help ()
{
    echo "Usage: run_buffered.sh --spec <name_spec.js>[,<name_spec.js>...] OPTIONS"
    echo "Runs the specified cypress tests in one cypress process, with the"
    echo "output buffered to a log file"
    echo ""
    echo "   --spec <files>             The test files to run, separated by commas"
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
      --second-chance)    SECOND_CHANCE=true;;
      --help)             print_help ;;
  -*) ;; # ignore
  esac
  shift
done
TEST_ERROR="${TEST_LOG}.error"

# Cypress appends a line here for each spec it finishes.
SPEC_RESULTS="${TEST_LOG}.results"

TEST_FOLDER=
if [ "${TEST_TYPE}" = "desktop" -o "${TEST_TYPE}" = "interfer-desktop" ]; then
    TEST_FOLDER=${DESKTOP_TEST_FOLDER};
elif [ "${TEST_TYPE}" = "mobile" -o "${TEST_TYPE}" = "interfer-mobile" ]; then
    TEST_FOLDER=${MOBILE_TEST_FOLDER};
elif [ "${TEST_TYPE}" = "idle" ]; then
    TEST_FOLDER=${IDLE_TEST_FOLDER};
elif [ "${TEST_TYPE}" = "multi-user" ]; then
    TEST_FOLDER=${MULTIUSER_TEST_FOLDER};
elif [ "${TEST_TYPE}" = "lighthouse" ]; then
    TEST_FOLDER=${LIGHTHOUSE_TEST_FOLDER};
elif [ "${TEST_TYPE}" = "interfer" ]; then
    TEST_FOLDER="${DIR}/integration_tests/common/";
fi

IFS=',' read -r -a SPECS <<< "${TEST_FILE}"

SETSID_WAIT=
if setsid --wait true 2>/dev/null; then
    SETSID_WAIT="--wait"
fi

run_command ()
{
    local paths=
    local spec=
    for spec in "$@"; do
        paths="${paths}${paths:+,}${TEST_FOLDER}${spec}"
    done
    echo "setsid ${SETSID_WAIT} ${CYPRESS_BINARY} run \
    --browser ${BROWSER} \
    --headless \
    --config-file ${TEST_CONFIG_FILE}\
    --config ${TEST_CONFIG}\
    --env ${TEST_ENV}\
    --spec=${paths}"
}

# Run the specs in one cypress process, appending the output to the log and
# leaving ${TEST_ERROR} behind when the process failed.
run_specs ()
{
    local command=
    command=$(run_command "$@")
    rm -rf ${TEST_ERROR}
    echo "`echo ${command} && ${command} || touch ${TEST_ERROR}`" >> ${TEST_LOG} 2>&1
}

# The start, end and outcome cypress recorded for one spec, if it ran. The
# recorded path is relative to the cypress project, so match it on the
# suite folder and spec.
spec_result ()
{
    test -f ${SPEC_RESULTS} || return
    awk -v spec="${TEST_FOLDER#${DIR}/}$1" '
        { tail = substr($4, length($4) - length(spec))
          if ($4 == spec || tail == "/" spec) { start = $1; end = $2; state = $3 } }
        END { if (state != "") print start, end, state }' ${SPEC_RESULTS}
}

print_error() {
    SPEC=$1
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
    elif [[ $1 == *"user1"* ]]; then
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
rm -rf ${TEST_ERROR} ${SPEC_RESULTS}
: > ${TEST_LOG}
export COOL_SPEC_RESULTS_FILE=${SPEC_RESULTS}

declare -A SPEC_START
declare -A SPEC_END
declare -A SPEC_RETRIED

RUN_START=`date +%s`
run_specs "${SPECS[@]}"
RUN_END=`date +%s`

# Collect the specs to run again: the ones that failed, and the ones that
# never started, since a failure takes the whole process down with it.
RETRY_SPECS=()
FAILED_IN_RUN=()
for spec in "${SPECS[@]}"; do
    read -r start end state <<< "$(spec_result ${spec})"
    if [ -n "${state}" ]; then
        SPEC_START[${spec}]=${start}
        SPEC_END[${spec}]=${end}
    fi
    if [ "${state}" = "failed" ]; then
        FAILED_IN_RUN+=("${spec}")
    fi
    if [ -f ${TEST_ERROR} ] && [ "${state}" != "passed" ]; then
        RETRY_SPECS+=("${spec}")
    fi
done

# A process that failed blaming no spec leaves all its specs to run again.
if [ -f ${TEST_ERROR} ] && [ ${#RETRY_SPECS[@]} -eq 0 ]; then
    RETRY_SPECS=("${SPECS[@]}")
fi

# Cypress's own retries only re-run failed test bodies, not whole-spec
# failures like a hook error or a process that dies at launch. Run each spec
# that has not passed once more, on its own.
FAILED_SPECS=()
if [ ${#RETRY_SPECS[@]} -gt 0 ] && [ ${SECOND_CHANCE} = true ]; then
    for spec in "${RETRY_SPECS[@]}"; do
        echo "Second chance: ${spec}" >> ${TEST_LOG}
        retry_start=`date +%s`
        run_specs "${spec}"
        retry_end=`date +%s`
        if [ -n "${SPEC_START[${spec}]:-}" ]; then
            SPEC_RETRIED[${spec}]=yes
        else
            SPEC_START[${spec}]=${retry_start}
        fi
        SPEC_END[${spec}]=${retry_end}
        if [ -f ${TEST_ERROR} ]; then
            FAILED_SPECS+=("${spec}")
        fi
    done
elif [ ${#RETRY_SPECS[@]} -gt 0 ]; then
    # Without a second chance the specs that never started are left unrun,
    # so name the ones that failed, or all of them where none was blamed.
    FAILED_SPECS=("${FAILED_IN_RUN[@]}")
    if [ ${#FAILED_SPECS[@]} -eq 0 ]; then
        FAILED_SPECS=("${SPECS[@]}")
    fi
fi

# A spec's console output is flushed in one block when it finishes, so the
# times from the plugin are the only per-spec ones. A spec that never ran
# takes the whole run's times.
for spec in "${SPECS[@]}"; do
    start=${SPEC_START[${spec}]:-${RUN_START}}
    end=${SPEC_END[${spec}]:-${RUN_END}}
    printf '%s %s %s %s %s %s\n' "${start}" "${end}" "$((end - start))" \
        "${SPEC_RETRIED[${spec}]:-no}" "${TEST_TYPE}" "${spec}" >> ${SPEC_TIMINGS}
done

if [ ${#FAILED_SPECS[@]} -eq 0 ];
    then cat ${TEST_LOG};
    else for spec in "${FAILED_SPECS[@]}"; do
             echo -e "Cypress test failed: ${spec}\n"
         done && \
         cat ${TEST_LOG} >> ${ERROR_LOG} && \
         for spec in "${FAILED_SPECS[@]}"; do
             print_error "${spec}"
         done;
fi;

# vim:set shiftwidth=4 expandtab:
