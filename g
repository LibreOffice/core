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
    echo "   -1         report the repos name on the first line of the output as <repo>:"
    echo "   -z         just to some house cleaning (hooks mostly). this is a stand-alone option as in ./g -z"
    echo "   --set-push-user [username] re-write an existing tree's config with an fd.o commit account name"
    exit $?
fi


pushd $(dirname $0) > /dev/null
COREDIR=$(pwd)
popd > /dev/null

refresh_hooks()
{
    repo=$1
    case "$repo" in
	core)
	    pushd $COREDIR > /dev/null
	    for hook_name in $(ls -1 $COREDIR/git-hooks) ; do
		hook=".git/hooks/$hook_name"
		if [ ! -x "$hook" -a ! -L "$hook" ] ; then
		    rm -f "$hook"
		    ln -s "git-hooks/$hook_name" "$hook"
		fi
	    done
	    popd > /dev/null
	    ;;
	translations)
	    if [ -d $COREDIR/clone/translations ] ; then
		pushd $COREDIR/clone/translations > /dev/null
		for hook_name in $(ls -1 $COREDIR/clone/translations/git-hooks) ; do
		    hook=".git/hooks/$hook_name"
		    rm -f "$hook"
		    ln -sf "git-hooks/$hook_name" "$hook"
		done
		# .gitattribute should be per-repo, avoid entangling repos
		if [ -L .gitattributes ] ; then
		    rm -f .gitattributes
		fi
		popd > /dev/null
	    fi
	    ;;
	help|dictionaries)
	    if [ -d $COREDIR/clone/$repo ] ; then
		pushd $COREDIR/clone/$repo > /dev/null
		# fixme: we should really keep these per-repo to
		# keep the repos independant. since these two
		# are realy not independant yet, we keep using core's hooks
		for hook_name in $(ls -1 $COREDIR/git-hooks) ; do
		    hook=".git/hooks/$hook_name"
		    rm -f "$hook"
		    ln -sf "$COREDIR/git-hooks/$hook_name" "$hook"
		done
		# .gitattribute should be per-repo, avoid entangling repos
		if [ -L .gitattributes ] ; then
		    rm -f .gitattributes
		fi
		popd > /dev/null
	    fi
	    ;;
    esac
}

refresh_all_hooks()
{
    repos="core $(cat "$COREDIR/bin/repo-list")"
    for repo in $repos ; do
	refresh_hooks $repo
    done
}

postprocess()
{
    rc=$1
    if $DO_HOOK_REFRESH ; then
	refresh_all_hooks
    fi

    exit $rc;
}

CLONEDIR="$COREDIR/clone"
if [ ! -e ${CLONEDIR} ]; then mkdir -p "$CLONEDIR"; fi

# extra params for some commands, like log
EXTRA=
COMMAND="$1"
PAGER=
RELATIVIZE=1
PUSH_ALL=
ALLOW_EMPTY=
KEEP_GOING=0
REPORT_REPOS=1
REPORT_COMPACT=0
DO_HOOK_REFRESH=false

while [ "${COMMAND:0:1}" = "-" ] ; do
    case "$COMMAND" in
        -f) KEEP_GOING=1
            ;;
        -s) REPORT_REPOS=0
            ;;
	-1) REPORT_COMPACT=1
            ;;
	--set-push-user)
	    shift
	    PUSH_USER="$1"
	    ;;
	-z)
	    DO_HOOK_REFRESH=true
	    postprocess 0
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
    clone|fetch|pull)
	DO_HOOK_REFRESH=true
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
DIRS="core $(cd $CLONEDIR ; ls)"
if [ "$COMMAND" = "clone" ] ; then
    DIRS=$(cat "$COREDIR/bin/repo-list")
fi
for REPO in $DIRS ; do
    DIR="$CLONEDIR/$REPO"
    NAME="$REPO"
    if [ "$REPO" = "core" ] ; then
        DIR="$COREDIR"
        NAME="main repo"
    fi

    if [ -d "$DIR" -a "z$PUSH_USER" != "z" ]; then
       echo "setting up push url for $DIR"
	   (cd $DIR && git config remote.origin.pushurl "ssh://${PUSH_USER}@git.freedesktop.org/git/libreoffice/${REPO}")
    elif [ \( -d "$DIR" -a -d "$DIR"/.git \) -o \( "$COMMAND" = "clone" \) ] ; then
        (
            # executed in a subshell
            if [ "$COMMAND" != "clone" ] ; then
                cd "$DIR"
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
		    EXTRA=${EXTRA/core/${REPO}}
                    ;;
            esac

            # do it!
            if [ "$COMMAND" != "clone" -o ! -d $DIR ] ; then
                if [ "$REPORT_REPOS" = "1" -a "$COMMAND" != "grep" ] ; then
                    if [ "$REPORT_COMPACT" = "1" ] ; then
                        echo -n "${REPO}:"
                    else
                        echo "===== $NAME ====="
                    fi
                fi
                git $PAGER "$COMMAND" $EXTRA "${FILES[@]}"
                RETURN=$?
            fi

            # now we can change the dir in case of clone as well
            if [ "$COMMAND" = "clone" ] ; then
                cd $DIR
            fi

            case "$COMMAND" in
                pull|clone)
                    # update links
		    if [ "$DIR" != "$COREDIR" ]; then
			for link in $(ls) ; do
			    if [ ! -e "$COREDIR/$link" ] ; then
			        if test -h "$COREDIR/$link"; then
				    rm "$COREDIR/$link"
				    echo -n "re-"
			        fi
                                echo "creating missing link $link"
                                ln -s "$DIR/$link" "$COREDIR/$link"
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
        ) || postprocess $?
    fi
done

# Cleanup the broken links
if [ "$COMMAND" = "pull" ] ; then
    for link in $(ls $COREDIR) ; do
        if [ -h "$COREDIR/$link" -a ! -e "$COREDIR/$link" ]; then
            echo "Removing broken link $link"
            rm $COREDIR/$link
        fi
    done
fi

# warn
if [ "$COMMAND" = "apply" ] ; then
    echo
    echo "Don't forget to check the status & commit now ;-)"
    echo
fi

postprocess $?

# vi:set shiftwidth=4 expandtab:
