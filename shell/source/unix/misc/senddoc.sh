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

URI_ENCODE=$(dirname "$0")/uri-encode
FOPTS=""
sd_platform=$(uname -s)

# linux file utility needs -L option to resolve symlinks
if [ "$sd_platform" = "Linux" ] ; then
  FOPTS="-L"
fi

# do not confuse the system mail clients with OOo and Java libraries
unset LD_LIBRARY_PATH

if [ "$1" = "--mailclient" ]; then
    shift
    MAILER=$1
    shift
fi

# autodetect mail client from executable name

MAILER_TYPE=$(basename "$MAILER")
case $(printf %s "$MAILER_TYPE" | sed 's/-.*$//') in

    iceape | mozilla | netscape | seamonkey | icedove | thunderbird | betterbird)
        # find mozilla script in PATH if necessary
        if [ "$MAILER_TYPE" = "$MAILER" ]; then
            moz=$(command -v "$MAILER")
        else
            moz=$MAILER
        fi

        MAILER_TYPE=mozilla
        # checks for the original mozilla start script(s)
        # and restrict the "-remote" semantics to those.
        if file $FOPTS "$moz" | grep "script" > /dev/null && grep "[NM]PL" "$moz" > /dev/null; then
            "$moz" -remote 'ping()' 2>/dev/null >/dev/null
            if [ $? -ne 2 ]; then
                MAILER_TYPE=mozilla-remote
            fi
        fi
        ;;

    kmail)
        MAILER_TYPE=kmail
        ;;

    mutt)
        MAILER_TYPE=mutt
        ;;

    evolution | gnome | groupwise | xdg) # NB. shortened from the dash on
        if [ "$MAILER_TYPE" != "xdg-email" ]; then
            MAILER_TYPE=generic-mailto
        fi
        ;;

    dtmail)
        MAILER_TYPE=dtmail
        ;;

    sylpheed | claws)
        MAILER_TYPE=sylpheed
        ;;

    Mail | Thunderbird | Betterbird | *.app )
        MAILER_TYPE=apple
        ;;

    *)
        MAILER_TYPE=generic-mailto

        # LO is configured to use something we do not recognize, or is not configured.
        # Try to be smart, and send the mail anyway, if we have the
        # possibility to do so.

        if [ -x /usr/bin/xdg-email ] ; then
            MAILER=/usr/bin/xdg-email
        elif [ -n "$DESKTOP_LAUNCH" ]; then
            # http://lists.freedesktop.org/pipermail/xdg/2004-August/002873.html
            MAILER=${DESKTOP_LAUNCH}
        elif [ -n "$KDE_FULL_SESSION" -a -x /usr/bin/kde-open ] ; then
            MAILER=/usr/bin/kde-open
        elif [ -x /usr/bin/xdg-open ] ; then
            MAILER=/usr/bin/xdg-open
        elif command -v xdg-open >/dev/null 2>&1 ; then
            MAILER=$(command -v xdg-open)
        else
            echo "Unsupported mail client: $MAILER"
            exit 2
        fi

        if [ "$(basename "$MAILER")" = "xdg-email" ]; then
            MAILER_TYPE=xdg-email
        fi
        ;;
esac

case $MAILER_TYPE in

    mozilla | mozilla-remote)

        while [ "$1" != "" ]; do
            case $1 in
                --to)
                    TO=${TO:+${TO},}$2
                    shift
                    ;;
                --cc)
                    CC=${CC:+${CC},}$2
                    shift
                    ;;
                --bcc)
                    BCC=${BCC:+${BCC},}$2
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
                    ATTACH=${ATTACH:+${ATTACH},}$(printf file://%s "$2" | "${URI_ENCODE}")
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done

        if [ "$TO" != "" ]; then
            COMMAND=${COMMAND:+${COMMAND},}to=\'${TO}\'
        fi
        if [ "$CC" != "" ]; then
            COMMAND=${COMMAND:+${COMMAND},}cc=\'${CC}\'
        fi
        if [ "$BCC" != "" ]; then
            COMMAND=${COMMAND:+${COMMAND},}bcc=\'${BCC}\'
        fi
        if [ "$SUBJECT" != "" ]; then
            COMMAND=${COMMAND:+${COMMAND},}subject=\'${SUBJECT}\'
        fi
        if [ "$BODY" != "" ]; then
            COMMAND=${COMMAND:+${COMMAND},}body=\'${BODY}\'
        fi
        if [ "$ATTACH" != "" ]; then
            COMMAND=${COMMAND:+${COMMAND},}attachment=\'${ATTACH}\'
        fi

        if [ "$MAILER_TYPE" = "mozilla" ]; then
            "$MAILER" -compose "$COMMAND" &
        else
            "$MAILER" -remote "xfeDoCommand(composeMessage,$COMMAND)" &
        fi
        ;;

    kmail)

        while [ "$1" != "" ]; do
            case $1 in
                --to)
                    TO="${TO:+${TO},}$2"
                    shift
                    ;;
                --cc)
                    CC="${CC:+${CC},}$2"
                    shift
                    ;;
                --bcc)
                    BCC="${BCC:+${BCC},}$2"
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
                --from)
                    FROM="$2"
                    shift
                    ;;
                --attach)
                    ATTACH="${ATTACH:+${ATTACH} }--attach "$(printf file://%s "$2" | "${URI_ENCODE}")
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done

        ${MAILER} --composer \
            ${CC:+--cc "${CC}"} \
            ${BCC:+--bcc "${BCC}"} \
            ${SUBJECT:+--subject "${SUBJECT}"} \
            ${BODY:+--body "${BODY}"} \
            ${FROM:+--header "From: ${FROM}"} \
            ${ATTACH:+${ATTACH}}  \
            ${TO:+"${TO}"}
        ;;

    mutt)

        while [ "$1" != "" ]; do
            case $1 in
                --from)
                    FROM="$2"
                    shift
                    ;;
                --to)
                    TO="${TO:+${TO},}$2"
                    shift
                    ;;
                --cc)
                    CC="${CC:+${CC},}$2"
                    shift
                    ;;
                --bcc)
                    BCC="${BCC:+${BCC},}$2"
                    shift
                    ;;
                --subject)
                    SUBJECT="$2"
                    shift
                    ;;
                --body)
                    TEMPLATE="$(basename "$0").mutt.XXXXXXXX"
                    BODY=$(mktemp -q -t "${TEMPLATE}")
                    echo "$2" > "$BODY"
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
            ${FROM:+-e "set from=\"${FROM}\""} \
            ${CC:+-c "${CC}"} \
            ${BCC:+-b "${BCC}"} \
            ${SUBJECT:+-s "${SUBJECT}"} \
            ${BODY:+-i "${BODY}"} \
            ${ATTACH:+-a "${ATTACH}"} \
            ${TO:+"${TO}"} &
        rm -f "$BODY"
        ;;

    generic-mailto | xdg-email)

        while [ "$1" != "" ]; do
            case $1 in
                --to)
                    if [ "${TO}" != "" ]; then
                        MAILTO="${MAILTO:+${MAILTO}&}to=$2"
                    else
                        TO="$2"
                    fi
                    shift
                    ;;
                --cc)
                    MAILTO="${MAILTO:+${MAILTO}&}cc="$(printf %s "$2" | "${URI_ENCODE}")
                    shift
                    ;;
                --bcc)
                    MAILTO="${MAILTO:+${MAILTO}&}bcc="$(printf %s "$2" | "${URI_ENCODE}")
                    shift
                    ;;
                --subject)
                    MAILTO="${MAILTO:+${MAILTO}&}subject="$(printf %s "$2" | "${URI_ENCODE}")
                    shift
                    ;;
                --body)
                    MAILTO="${MAILTO:+${MAILTO}&}body="$(printf %s "$2" | "${URI_ENCODE}")
                    shift
                    ;;
                --attach)
                    # Just add both attach and attachment "headers" - some apps use one, some the other
                    ATTACH_URL=$(printf file://%s "$2" | "${URI_ENCODE}")
                    MAILTO="${MAILTO:+${MAILTO}&}attach=${ATTACH_URL}&attachment=${ATTACH_URL}"
                    if [ "$MAILER_TYPE" = "xdg-email" ]; then
                        # Also add the argument: see https://gitlab.freedesktop.org/xdg/xdg-utils/-/issues/177
                        ATTACH="${ATTACH:+${ATTACH} }--attach \"$2\""
                    fi
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done

        MAILTO="mailto:${TO}?${MAILTO}"
        eval "${MAILER} ${ATTACH} \"${MAILTO}\"" &
        ;;

    dtmail)

        while [ "$1" != "" ]; do
            case $1 in
                --to)
                    TO=${TO:+${TO},}$2
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

        ${MAILER} ${TO:+-T ${TO}} ${ATTACH:+-a "${ATTACH}"}
        ;;

    sylpheed)

        while [ "$1" != "" ]; do
            case $1 in
                --to)
                    TO=${TO:+${TO},}$2
                    shift
                    ;;
                --attach)
                    ATTACH=${ATTACH:+${ATTACH},}$(printf file://%s "$2" | "${URI_ENCODE}")
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done

         ${MAILER} ${TO:+--compose ${TO}} ${ATTACH:+--attach ${ATTACH}}
        ;;

    apple)

        while [ "$1" != "" ]; do
            case $1 in
                --attach)
                    #i95688# fix filenames containing accented chars, whatever alien
                    ATTACH="${ATTACH:+${ATTACH} }"$(printf file://%s "$2" | "${URI_ENCODE}")
                    shift
                    ;;
                *)
                    ;;
            esac
            shift;
        done
        /usr/bin/open -a "${MAILER}" ${ATTACH}
        ;;
esac
