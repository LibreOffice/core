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
    echo "   --set-push-user [username] re-write an existing tree's config with an fd.o commit account name"
    exit $?
fi

CLONEDIR=$(perl -e 'use Cwd "abs_path"; print abs_path(shift);' $0 | sed -e ' s/\/g$/\/clone/')
if [ ! -e ${CLONEDIR} ]; then mkdir -p $CLONEDIR; fi
RAWBUILDDIR=$(perl -e 'use Cwd "abs_path"; print abs_path(shift);' $0 | sed -e ' s/\/g$//')
if [ ! -e ${RAWBUILDDIR} ]; then mkdir -p $RAWBUILDDIR; fi

# should we need to update the hooks
function update_hooks()
{
    HOOKDIR="$1"
    for H in $(cd "$RAWBUILDDIR/git-hooks" ; echo *) ; do
        HOOK=".git/hooks/$H"
        if [ ! -x "$HOOK" -a ! -L "$HOOK" ] ; then
            rm -f "$HOOK"
            ln -s "$HOOKDIR/$H" "$HOOK"
        fi
    done
    if [ -z "$(git config push.default)" ] ; then
        git config push.default tracking
    fi
    if [ "$PWD" != "$RAWBUILDDIR" -a ! -e .gitattributes ]; then
        ln -s $RAWBUILDDIR/.gitattributes .
    fi
}

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
	--set-push-user)
	    shift
	    PUSH_USER="$1"
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
            if [ "$COMMAND" = "commit" -a "$PARAM" = "-F" ]
            then
                shift
                # this still needs some magic to handle relative paths
                EXTRA="${EXTRA} -F ${1}"
            else
                [ "$COMMAND" = "commit" -a "$PARAM" = "--allow-empty" ] && ALLOW_EMPTY=1
                FILES[$FILESNUM]="$PARAM"
                FILESNUM=$(($FILESNUM+1))
            fi
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
        FILES[$FILESNUM]=$(perl -e 'use Cwd "abs_path"; print abs_path(shift);' "$PARAM")
        if [ -z "${FILES[$FILESNUM]}" -o ! -e "${FILES[$FILESNUM]}" ] ; then
            # it is probably not a file, but a tag name, or something
            FILES[$FILESNUM]="$PARAM"
        fi
        FILESNUM=$(($FILESNUM+1))
    fi
done

# do it!
DIRS="bootstrap $(cd $CLONEDIR ; ls)"
if [ "$COMMAND" = "clone" ] ; then
    DIRS=$(cat "$RAWBUILDDIR/bin/repo-list")
    # update hooks in the main repo too
    ( cd "$RAWBUILDDIR" ; update_hooks "../../git-hooks" )
fi
for REPO in $DIRS ; do
    DIR="$CLONEDIR/$REPO"
    NAME="$REPO"
    HOOKDIR="../../../../git-hooks"
    if [ "$REPO" = "bootstrap" ] ; then
        DIR="$RAWBUILDDIR"
        NAME="main repo"
        HOOKDIR="../../git-hooks"
    fi

    if [ -d "$DIR" -a "z$PUSH_USER" != "z" ]; then
       echo "setting up push url for $DIR"
	   (cd $DIR && git config remote.origin.pushurl "ssh://${PUSH_USER}@git.freedesktop.org/git/libreoffice/${REPO}")
    elif [ \( -d "$DIR" -a -d "$DIR"/.git \) -o \( "$COMMAND" = "clone" \) ] ; then
        (
            # executed in a subshell
            if [ "$COMMAND" != "clone" ] ; then
                cd "$DIR"
                update_hooks "$HOOKDIR"
            else
                cd "$CLONEDIR"
            fi

            # relativize the absolutized params again if we want to operate
            # only on the files belonging to this exact repo
            if [ "$RELATIVIZE" = "1" -a -n "$FILES" ] ; then
                FILESNUM=0
                INSERTNUM=0
                PWD=$(pwd)
                PWDLEN=$(pwd | wc -c)
                for I in "${FILES[@]}" ; do
                    I="${I//@REPO@/${REPO}}"
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
                        [ -z "$(git diff-index --name-only HEAD --)" ] && exit 0
                    fi
                    ;;
                push)
                    if [ "$PUSH_ALL" != "1" ] ; then
                        [ -n "$(git rev-list origin..HEAD)" ] || exit 0
                    fi
                    ;;
                status)
                    LOCALCOMMITS="$(git rev-list origin..HEAD)"
                    if [ -z "$LOCALCOMMITS" ] ; then
                        [ -z "$(git diff-index --name-only HEAD --)" ] && exit 0
                    fi
                    ;;
                clone)
                    EXTRA="$(git config remote.origin.url)"
		    EXTRA=${EXTRA/bootstrap/${REPO}}
                    ;;
            esac

            # check for changes
            HEADREF=$(git show-ref --head HEAD)

            # do it!
	    if [ "$COMMAND" != "clone" -o ! -d $DIR ] ; then
                [ "$REPORT_REPOS" = "1" ] && echo "===== $NAME ====="
                git $PAGER "$COMMAND" $EXTRA "${FILES[@]}"
                RETURN=$?
	    fi

            # now we can change the dir in case of clone as well
            if [ "$COMMAND" = "clone" ] ; then
                cd $DIR
                update_hooks "$HOOKDIR"
            fi

            # update stamp if the repo changed
            NEWHEADREF=$(git show-ref --head HEAD)
            [ "$HEADREF" != "$NEWHEADREF" ] && touch $CLONEDIR/repos_changed

            case "$COMMAND" in
                pull|clone)
                    # update links
		    if [ "$DIR" != "$RAWBUILDDIR" ]; then
			for link in $(ls) ; do
			    if [ ! -e "$RAWBUILDDIR/$link" ] ; then
			        if test -h "$RAWBUILDDIR/$link"; then
				    rm "$RAWBUILDDIR/$link"
				    echo -n "re-"
			        fi
                                echo "creating missing link $link $DIR/$link -> $RAWBUILDDIR/$link"
                                ln -s "$DIR/$link" "$RAWBUILDDIR/$link"
                            fi
                        done
		    fi
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
    for link in $(ls $RAWBUILDDIR) ; do
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
