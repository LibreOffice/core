#!/bin/sh
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

URI_ENCODE="`dirname $0`/uri-encode"
FOPTS=""
sd_platform=`uname -s`

# linux file utility needs -L option to resolve symlinks
if [ "$sd_platform" = "Linux" ] ; then
  FOPTS="-L"
fi

# do not confuse the system mail clients with OOo and Java libraries
case $sd_platform in
  AIX)
    unset LIBPATH
    ;;
  *)
    unset LD_LIBRARY_PATH
    ;;
esac

# tries to locate the executable specified
# as first parameter in the user's path.
which() {
    if [ ! -z "$1" ]; then
        for i in `echo $PATH | sed -e 's/^:/.:/g' -e 's/:$/:./g' -e 's/::/:.:/g' -e 's/:/ /g'`; do
            if [ -x "$i/$1" -a ! -d "$i/$1" ]; then
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

    if file $FOPTS "$moz" | grep "script" > /dev/null && grep "[NM]PL" "$moz" > /dev/null; then
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

if [ "$1" = "--mailclient" ]; then
    shift
    MAILER=$1
    shift
fi

# autodetect mail client from executable name
case `basename "$MAILER" | sed 's/-.*$//'` in

    iceape | mozilla | netscape | seamonkey | icedove | thunderbird)

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
                    ATTACH=${ATTACH:-}${ATTACH:+,}`echo "file://$2" | ${URI_ENCODE}`
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
            COMMAND=${COMMAND:-}${COMMAND:+,}attachment=\'${ATTACH}\'
        fi

        run_mozilla "$MAILER" "$COMMAND"
        ;;

    kmail)

        while [ "$1" != "" ]; do
            case $1 in
                --to)
                    TO="${TO:-}${TO:+,}$2"
                    shift
                    ;;
                --cc)
                    CC="${CC:-}${CC:+,}$2"
                    shift
                    ;;
                --bcc)
                    BCC="${BCC:-}${BCC:+,}$2"
                    shift
                    ;;
                --subject)
                    SUBJECT="$2"
                    shift
                    ;;
                --body)
                    BODY="$2"
                    shift
                    ;;
                --attach)
                    ATTACH="$2"
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done

        ${MAILER} --composer ${CC:+--cc} ${CC:+"${CC}"} ${BCC:+--bcc} ${BCC:+"${BCC}"} \
            ${SUBJECT:+--subject} ${SUBJECT:+"${SUBJECT}"} ${BODY:+--body} ${BODY:+"${BODY}"} \
            ${ATTACH:+--attach} ${ATTACH:+"${ATTACH}"} ${TO:+"${TO}"}
        ;;

    mutt)

        while [ "$1" != "" ]; do
            case $1 in
                --from)
                    FROM="$2"
                    shift
                    ;;
                --to)
                    TO="${TO:-}${TO:+,}$2"
                    shift
                    ;;
                --cc)
                    CC="${CC:-}${CC:+,}$2"
                    shift
                    ;;
                --bcc)
                    BCC="${BCC:-}${BCC:+,}$2"
                    shift
                    ;;
                --subject)
                    SUBJECT="$2"
                    shift
                    ;;
                --body)
                    TEMPLATE="`basename $0`.mutt.XXXXXXXX"
                    BODY=`mktemp -q -t ${TEMPLATE}`
                    echo "$2" > $BODY
                    shift
                    ;;
                --attach)
                    ATTACH="$2"
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done

        x-terminal-emulator -e ${MAILER} \
            ${FROM:+-e} ${FROM:+"set from=\"${FROM}\""} \
            ${CC:+-c} ${CC:+"${CC}"} \
            ${BCC:+-b} ${BCC:+"${BCC}"} \
            ${SUBJECT:+-s} ${SUBJECT:+"${SUBJECT}"} \
            ${BODY:+-i} ${BODY:+"${BODY}"} \
            ${ATTACH:+-a} ${ATTACH:+"${ATTACH}"} \
            ${TO:+"${TO}"} &
        rm -f $BODY
        ;;

    evolution | gnome | xdg) # NB. shortened from the dash on

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
                    MAILTO="${MAILTO:-}${MAILTO:+&}cc="`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --bcc)
                    MAILTO="${MAILTO:-}${MAILTO:+&}bcc="`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --subject)
                    MAILTO="${MAILTO:-}${MAILTO:+&}subject"=`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --body)
                    MAILTO="${MAILTO:-}${MAILTO:+&}body="`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --attach)
                    MAILTO="${MAILTO:-}${MAILTO:+&}attach="`echo "file://$2" | ${URI_ENCODE}`
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

    groupwise)

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
                    MAILTO="${MAILTO:-}${MAILTO:+&}cc="`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --bcc)
                    MAILTO="${MAILTO:-}${MAILTO:+&}bcc="`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --subject)
                    MAILTO="${MAILTO:-}${MAILTO:+&}subject"=`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --body)
                    MAILTO="${MAILTO:-}${MAILTO:+&}body="`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --attach)
                    MAILTO="${MAILTO:-}${MAILTO:+&}attachment="`echo "file://$2" | ${URI_ENCODE}`
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

    dtmail)

        while [ "$1" != "" ]; do
            case $1 in
                --to)
                    TO=${TO:-}${TO:+,}$2
                    shift
                    ;;
                --attach)
                    ATTACH="$2"
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done

        ${MAILER} ${TO:+-T} ${TO:-} ${ATTACH:+-a} ${ATTACH:+"${ATTACH}"}
        ;;

    sylpheed | claws)

        while [ "$1" != "" ]; do
            case $1 in
                --to)
                    TO=${TO:-}${TO:+,}$2
                    shift
                    ;;
                --attach)
                    ATTACH="${ATTACH:-}${ATTACH:+ }$2"
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done

         ${MAILER} ${TO:+--compose} "${TO:-}" ${ATTACH:+--attach} "${ATTACH:-}"
        ;;

    Mail | Thunderbird | *.app )

        while [ "$1" != "" ]; do
            case $1 in
                --attach)
                    #i95688# fix filenames containing accented chars, whatever alien
                    ATTACH="${ATTACH:-}${ATTACH:+ }"`echo "file://$2" | ${URI_ENCODE}`
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done
        /usr/bin/open -a "${MAILER}" ${ATTACH}
        ;;

    *)

        # LO is configured to use something we do not recognize, or is not configured.
        # Try to be smart, and send the mail anyway, if we have the
        # possibility to do so.

        if [ -n "$DESKTOP_LAUNCH" ]; then
            # http://lists.freedesktop.org/pipermail/xdg/2004-August/002873.html
            MAILER=${DESKTOP_LAUNCH}
        elif [ -n "$KDE_FULL_SESSION" -a -x /usr/bin/kde-open ] ; then
            MAILER=/usr/bin/kde-open
        elif [ -n "$GNOME_DESKTOP_SESSION_ID" -a -x /usr/bin/gnome-open ] ; then
            MAILER=/usr/bin/gnome-open
        elif [ -x /usr/bin/xdg-open ] ; then
            MAILER=/usr/bin/xdg-open
        else
            echo "Unsupported mail client: `basename $MAILER | sed 's/-.*^//'`"
            exit 2
        fi

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
                    MAILTO="${MAILTO:-}${MAILTO:+&}cc="`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --bcc)
                    MAILTO="${MAILTO:-}${MAILTO:+&}bcc="`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --subject)
                    MAILTO="${MAILTO:-}${MAILTO:+&}subject"=`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --body)
                    MAILTO="${MAILTO:-}${MAILTO:+&}body="`echo "$2" | ${URI_ENCODE}`
                    shift
                    ;;
                --attach)
                    MAILTO="${MAILTO:-}${MAILTO:+&}attachment="`echo "file://$2" | ${URI_ENCODE}`
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
esac

exit 0
