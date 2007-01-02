#!/bin/sh

jarfilename="JavaSetup.jar"
java_runtime="java"
java_runtime_set="no"
java_runtime_found="no"

# the user might want to specify java runtime on the commandline
USAGE="Usage: $0 [ -j <java_runtime> ]"
while getopts hj: opt; do
    case $opt in
        j)  java_runtime_set="yes";
            java_runtime="${OPTARG}"
            if [ ! -f "$java_runtime" ]; then
                echo "Invalid java runtime $java_runtime, file does not exist"
                exit 1
            fi
            if [ ! -x "$java_runtime" ]; then
                echo "Invalid java runtime $java_runtime, not an executable file"
                exit 1
            fi
            ;;
        h)  echo ${USAGE}
            exit 1
            ;;
        \?) echo ${USAGE}
            exit 1
            ;;
    esac
done

# Search for Java, if not specified on command line

if [ "$java_runtime_set" != "yes" ]; then
    # searching for java runtime in path
    for i in `echo $PATH | sed -e 's/^:/.:/g' -e 's/:$/:./g' -e 's/::/:.:/g' -e 's/:/ /g'`; do
        if [ -x "$i/$java_runtime" -a ! -d "$i/$java_runtime" ]; then
            java_runtime="$i/$java_runtime"
            java_runtime_found="yes"
            break;
        fi
    done

    if [ "$java_runtime_found" != "yes" ]; then
        echo "No Java runtime environment found. Please install Java runtime before starting $0."
        exit 1
    fi
fi

# check Java version (not required yet) -> every Java has to work without problems

# Determining current platform
# platform=`uname -s`

# verify that we are running on the right platform -> has to be done in Java program.

umask 022

echo "Using $java_runtime"
echo `$java_runtime -version`
echo "Running installer"

# looking for environment variables

home=""
if [ "x" != "x$HOME" ]; then
    home=-DHOME=$HOME
fi

log_module_states=""
if [ "x" != "x$LOG_MODULE_STATES" ]; then
    log_module_states=-DLOG_MODULE_STATES=$LOG_MODULE_STATES
fi

getuid_path=""
if [ "x" != "x$GETUID_PATH" ]; then
    getuid_path=-DGETUID_PATH=$GETUID_PATH
fi

# changing into setup directory
cd "`dirname "$0"`"

# run the installer class file
echo $java_runtime $home $log_module_states $setup_root_path $package_root_path -jar $jarfilename
$java_runtime $home $log_module_states $getuid_path -jar $jarfilename

exit 0
