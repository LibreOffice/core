#!/bin/sh

# tries to locate the executable specified
# as first parameter in the user's path.
which() {
    if [ ! -z "$1" ]; then
        for i in `echo $PATH | sed -e 's/^:/.:/g' -e 's/:$/:./g' -e 's/::/:.:/g' -e 's/:/ /g'`; do
            if [ -x "$i/$1" ]; then
                echo "$i/$1"
                break;
            fi
        done
    fi
}

# checks for the original mozilla start script(s)
# and restrict the "-remote" semantics to those.
run_mozilla() {
    # find mozilla script in PATH if necessary
    if [ "`basename $1`" = "$1" ]; then
        moz=`which $1`
    else
        moz=$1
    fi

    if file "$moz" | grep "script" > /dev/null && grep "NPL" "$moz" > /dev/null; then
        "$moz" -remote 'ping()' 2>/dev/null >/dev/null
        if [ $? -eq 2 ]; then
            "$1" -compose "$2" &
        else
            "$1" -remote "xfeDoCommand(composeMessage,$2)" &
        fi
    else
        "$1" -compose "$2" &
    fi
}

# restore search path for dynamic loader to system defaults to
# avoid version clashes of mozilla libraries shipped with OOo
case `uname -s` in
    AIX)
        LIBPATH=$SYSTEM_LIBPATH
        if [ -z "$LIBPATH" ]; then
            unset LIBPATH SYSTEM_LIBPATH
        else
            export LIBPATH; unset SYSTEM_LIBPATH
        fi
        ;;

    Darwin)
        DYLD_LIBRARY_PATH=$SYSTEM_DYLD_LIBRARY_PATH
        if [ -z "$DYLD_LIBRARY_PATH" ]; then
            unset DYLD_LIBRARY_PATH SYSTEM_DYLD_LIBRARY_PATH
        else
            export DYLD_LIBRARY_PATH; unset SYSTEM_DYLD_LIBRARY_PATH
        fi
        ;;

    HP-UX)
        SHLIB_PATH=$SYSTEM_SHLIB_PATH
        if [ -z "$SHLIB_PATH" ]; then
            unset SHLIB_PATH SYSTEM_SHLIB_PATH
        else
            export SHLIB_PATH; unset SYSTEM_SHLIB_PATH
        fi
        ;;

    IRIX*)
        LD_LIBRARYN32_PATH=$SYSTEM_LD_LIBRARYN32_PATH
        if [ -z "$LD_LIBRARYN32_PATH" ]; then
            unset LD_LIBRARYN32_PATH SYSTEM_LD_LIBRARYN32_PATH
        else
            export LD_LIBRARYN32_PATH; unset $SYSTEM_LD_LIBRARYN32_PATH
        fi
        ;;

    *)
        LD_LIBRARY_PATH=$SYSTEM_LD_LIBRARY_PATH
        if [ -z "$LD_LIBRARY_PATH" ]; then
            unset LD_LIBRARY_PATH SYSTEM_LD_LIBRARY_PATH
        else
            export LD_LIBRARY_PATH; unset SYSTEM_LD_LIBRARY_PATH
        fi
        ;;
esac

if [ "$1" = "--mailclient" ]; then
    shift
    MAILER=$1
    shift
fi

# autodetect mail client from executable name
case `basename $MAILER | sed 's/-.*$//'` in

    mozilla | netscape | thunderbird)

        while [ "$1" != "" ]; do
            case $1 in
                --to)
                    TO=${TO:-}${TO:+,}$2
                    shift
                    ;;
                --cc)
                    CC=${CC:-}${CC:+,}$2
                    shift
                    ;;
                --bcc)
                    BCC=${BCC:-}${BCC:+,}$2
                    shift
                    ;;
                --subject)
                    SUBJECT=$2
                    shift
                    ;;
                --body)
                    BODY=$2
                    shift
                    ;;
                --attach)
                    ATTACH=${ATTACH:-}${ATTACH:+,}$2
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done

        if [ "$TO" != "" ]; then
            COMMAND=${COMMAND:-}${COMMAND:+,}to=${TO}
        fi
        if [ "$CC" != "" ]; then
            COMMAND=${COMMAND:-}${COMMAND:+,}cc=${CC}
        fi
        if [ "$BCC" != "" ]; then
            COMMAND=${COMMAND:-}${COMMAND:+,}bcc=${BCC}
        fi
        if [ "$SUBJECT" != "" ]; then
            COMMAND=${COMMAND:-}${COMMAND:+,}subject=${SUBJECT}
        fi
        if [ "$BODY" != "" ]; then
            COMMAND=${COMMAND:-}${COMMAND:+,}body=${BODY}
        fi
        if [ "$ATTACH" != "" ]; then
            COMMAND=${COMMAND:-}${COMMAND:+,}attachment=${ATTACH}
        fi

        run_mozilla "$MAILER" "$COMMAND"
        ;;

    kmail)

        while [ "$1" != "" ]; do
            case $1 in
                --to)
                    TO=${TO:-}${TO:+,}$2
                    shift
                    ;;
                --cc)
                    CC=${CC:-}${CC:+,}$2
                    shift
                    ;;
                --bcc)
                    BCC=${BCC:-}${BCC:+,}$2
                    shift
                    ;;
                --subject)
                    SUBJECT=$2
                    shift
                    ;;
                --body)
                    BODY=$2
                    shift
                    ;;
                --attach)
                    ATTACH="${ATTACH:-}${ATTACH:+ }--attach $2"
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done

        ${MAILER} --composer ${CC:+--cc} "${CC:-}" ${BCC:+--bcc} "${BCC:-}" ${SUBJECT:+--subject} \
            "${SUBJECT:-}" ${BODY:+--body} "${BODY}" ${ATTACH} "${TO}"
        ;;

    evolution)

        while [ "$1" != "" ]; do
            case $1 in
                --to)
                    if [ "${TO}" != "" ]; then
                        MAILTO="${MAILTO:-}${MAILTO:+&}to=$2"
                    else
                        TO="$2"
                    fi
                    shift
                    ;;
                --cc)
                    MAILTO=${MAILTO:-}${MAILTO:+&}cc=$2
                    shift
                    ;;
                --bcc)
                    MAILTO=${MAILTO:-}${MAILTO:+&}bcc=$2
                    shift
                    ;;
                --subject)
                    MAILTO=${MAILTO:-}${MAILTO:+&}subject=$2
                    shift
                    ;;
                --body)
                    MAILTO=${MAILTO:-}${MAILTO:+&}body=$2
                    shift
                    ;;
                --attach)
                    MAILTO=${MAILTO:-}${MAILTO:+&}attach=$2
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done

        MAILTO="mailto:${TO}?${MAILTO}"
        ${MAILER} "${MAILTO}" &
        ;;

# FIXME: dtmail dows not accept file urls, so we need to convert them
# to system paths !
#   dtmail)
#
#       while [ "$1" != "" ]; do
#           case $1 in
#               --to)
#                   TO=${TO:-}${TO:+,}$2
#                   shift
#                   ;;
#               --attach)
#                    ATTACH="${ATTACH:-}${ATTACH:+ } $2"
#                   shift
#                   ;;
#               *)
#                   ;;
#           esac
#           shift;
#       done
#
#        ${MAILER} ${TO:+-T} ${TO:-} ${ATTACH:+-a} ${ATTACH:-}
#       ;;
    *)
        if [ "$MAILER" != "" ]; then
            echo "Unsupported mail client: `basename $MAILER | sed 's/-.*^//'`"
        else
            echo "Could not determine a mail client to use."
        fi
        exit 2
        ;;
esac

exit 0
