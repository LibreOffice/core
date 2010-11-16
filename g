#!/usr/bin/env bash
#
# Wrapper for git to handle more subdirs at the same time
#

# no params, no action
if [ "$#" -eq "0" ] ; then
    git
    echo
    echo "Additional options available only in this 'g' wrapper:"
    echo
    echo "Usage: g [options] [git commands]"
    echo "   -f         Force - act on all the repos, not only the changed ones"
    echo "   -s         Silent - do not report the repo names."
    exit $?
fi

CLONEDIR=`perl -e 'use Cwd "abs_path"; print abs_path(shift);' $0 | sed -e ' s/\/g$/\/clone/'`
if [ ! -e ${CLONEDIR} ]; then mkdir -p $CLONEDIR; fi
RAWBUILDDIR=`perl -e 'use Cwd "abs_path"; print abs_path(shift);' $0 | sed -e ' s/\/g$//'`
if [ ! -e ${RAWBUILDDIR} ]; then mkdir -p $RAWBUILDDIR; fi

# extra params for some commands, like log
EXTRA=
COMMAND="$1"
PAGER=
RELATIVIZE=1
PUSH_ALL=
ALLOW_EMPTY=
KEEP_GOING=0
REPORT_REPOS=1

while [ "${COMMAND:0:1}" = "-" ] ; do
    case "$COMMAND" in
        -f) KEEP_GOING=1
            ;;
        -s) REPORT_REPOS=0
            ;;
    esac
    shift
    COMMAND="$1"
done

case "$COMMAND" in
    apply)
        EXTRA="-p0 --stat --apply --index --ignore-space-change --whitespace=error"
        RELATIVIZE=0
        ;;
    diff)
        PAGER='--no-pager'
        REPORT_REPOS=0
        ;;
    log)
        if [ "$#" = "1" ] ; then
            EXTRA='-1'
        fi
        PAGER='--no-pager'
        ;;
    push)
        if [ "$#" != "1" ] ; then
            PUSH_ALL=1
        fi
        ;;
esac

# absolutize the parameters first
unset FILES
FILESNUM=0
while shift ; do
    PARAM="$1"
    if [ -z "$PARAM" ] ; then
        continue
    elif [ "${PARAM:0:1}" = "-" ] ; then
        if [ \( "$COMMAND" = "checkout" -a "$PARAM" = "-b" \) -o \
             \( "$COMMAND" = "clone"    -a "$PARAM" = "--reference" \) -o \
             \( "$COMMAND" = "commit"   -a "$PARAM" = "-m" \) -o \
             \( "$COMMAND" = "commit"   -a "$PARAM" = "-am" \) -o \
             \( "$COMMAND" = "tag"      -a "$PARAM" = "-m" \) ]
        then
            # params that take an argument
            FILES[$FILESNUM]="$PARAM"
            FILESNUM=$(($FILESNUM+1))
            shift
            FILES[$FILESNUM]="$1"
            FILESNUM=$(($FILESNUM+1))
        else
            [ "$COMMAND" = "commit" -a "$PARAM" = "--allow-empty" ] && ALLOW_EMPTY=1

            FILES[$FILESNUM]="$PARAM"
            FILESNUM=$(($FILESNUM+1))
        fi
    else
        if [ "$COMMAND" = "apply" ] ; then
            grep -qs $'^+ *\t' "$PARAM" && {
                echo "Patch '$PARAM' introduces tabs in indentation, aborting."
                echo
                echo "Please fix the patch (something like s/^\(+ *\)\t/\1    /) and try again."
                echo
                exit 1
            }
        fi

        # make the paths absolute
        FILES[$FILESNUM]=`perl -e 'use Cwd "abs_path"; print abs_path(shift);' "$PARAM"`
        if [ -z "${FILES[$FILESNUM]}" -o ! -e "${FILES[$FILESNUM]}" ] ; then
            # it is probably not a file, but a tag name, or something
            FILES[$FILESNUM]="$PARAM"
        fi
        FILESNUM=$(($FILESNUM+1))
    fi
done

# do it!
cd "$CLONEDIR"
DIRS=". `ls`"
if [ "$COMMAND" = "clone" ] ; then
    DIRS="artwork base calc components extensions extras filters
          help impress libs-core libs-extern libs-extern-sys libs-gui
          postprocess sdk testing ure writer"
fi
for DIR in $DIRS ; do
    if [ \( -d "$DIR" -a -d "$DIR"/.git \) -o \( "$COMMAND" = "clone" \) ] ; then
        (
            # executed in a subshell
            [ "$COMMAND" != "clone" ] && cd $DIR

            # relativize the absolutized params again if we want to operate
            # only on the files belonging to this exact repo
            if [ "$RELATIVIZE" = "1" -a -n "$FILES" ] ; then
                FILESNUM=0
                INSERTNUM=0
                PWD=`pwd`
                PWDLEN=`pwd | wc -c`
                for I in "${FILES[@]}" ; do
                    I="${I//@REPO@/${DIR}}"
                    unset FILES[$FILESNUM]
                    FILESNUM=$(($FILESNUM+1))
                    # filter out files that don't belong to this repo
                    if [ \( "${I:0:1}" = "/" \) -a \( "$COMMAND" != "clone" \) ] ; then
                        if [ "${I:0:$PWDLEN}" = "$PWD/" ] ; then
                            FILES[$INSERTNUM]="${I:$PWDLEN}"
                            INSERTNUM=$(($INSERTNUM+1))
                        fi
                    else
                        FILES[$INSERTNUM]="$I"
                        INSERTNUM=$(($INSERTNUM+1))
                    fi
                done
                [ "$INSERTNUM" = "0" ] && exit 0
            fi

            # some extra params
            case "$COMMAND" in
                apply)
                    for I in * ; do
                        if [ -d "$I" ] ; then
                            EXTRA="$EXTRA --include=$I/*"
                        else
                            EXTRA="$EXTRA --include=$I"
                        fi
                    done
                    ;;
                commit)
                    if [ "$ALLOW_EMPTY" != "1" ] ; then
                        [ -z "`git diff-index --name-only HEAD --`" ] && exit 0
                    fi
                    ;;
                push)
                    if [ "$PUSH_ALL" != "1" ] ; then
                        [ -n "`git rev-list origin..HEAD`" ] || exit 0
                    fi
                    ;;
                status)
                    LOCALCOMMITS="`git rev-list origin..HEAD`"
                    if [ -z "$LOCALCOMMITS" ] ; then
                        [ -z "`git diff-index --name-only HEAD --`" ] && exit 0
                    fi
                    ;;
                clone)
                    if [ -z "$FILES" ]; then
                        EXTRA="$(git config remote.origin.url|sed 's|/[^/]\+$||')/${DIR}"
                    fi
                    ;;
            esac

            [ "$REPORT_REPOS" = "1" ] && echo "===== $DIR ====="

            # check for changes
            HEADREF=`git show-ref --head HEAD`

            # do it!
            git $PAGER "$COMMAND" $EXTRA "${FILES[@]}"
            RETURN=$?

            # now we can change the dir in case of clone as well
            [ "$COMMAND" = "clone" ] && cd $DIR

            # update stamp if the repo changed
            NEWHEADREF=`git show-ref --head HEAD`
            [ "$HEADREF" != "$NEWHEADREF" ] && touch $CLONEDIR/repos_changed

            case "$COMMAND" in
                pull|clone)
                    # update links
                    for link in `ls` ; do
                        if [ ! -e "$RAWBUILDDIR/$link" ] ; then
                            echo "Creating missing link $link"
                            ln -s "$CLONEDIR/$DIR/$link" "$RAWBUILDDIR/$link"
                        fi
                    done
                    ;;
                status)
                    # git status returns error in some versions, clear that
                    RETURN=0
                    ;;
                grep)
                    # git grep return an 'error' if nothing is found
                    # still we should continue grepping the other repos
                    RETURN=0
                    ;;
            esac
            if [  "$KEEP_GOING" = "1" ] ; then
                RETURN=0
            fi

            exit $RETURN
        ) || exit $?
    fi
done

# Cleanup the broken links
if [ "$COMMAND" = "pull" ] ; then
    for link in `ls $RAWBUILDDIR` ; do
        if [ -h "$RAWBUILDDIR/$link" -a ! -e "$RAWBUILDDIR/$link" ]; then
            echo "Removing broken link $link"
            rm $RAWBUILDDIR/$link
        fi
    done
fi

# warn
if [ "$COMMAND" = "apply" ] ; then
    echo
    echo "Don't forget to check the status & commit now ;-)"
    echo
fi

# vi:set shiftwidth=4 expandtab:
