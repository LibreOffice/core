#!/usr/bin/env bash
#
# Wrapper for git to handle more subdirs at the same time
#

if [ -n "$g_debug" ] ; then
    set -x
fi

SUBMODULES_ALL="dictionaries helpcontent2 translations"

pushd $(dirname $0) > /dev/null
if [ -f config_host.mk ] ; then
    # we are in the BUILDDIR
    SRC_ROOT=$(cat config_host.mk | grep -a SRC_ROOT | sed -e "s/.*=//")
else
    SRC_ROOT=$(pwd)
fi
popd > /dev/null

COREDIR="$SRC_ROOT"

usage()
{
    git
    echo
    echo "Usage: g [options] [git (checkout|clone|fetch|grep|pull|push|reset) [git options/args..]]"
    echo ""
    echo " -z restore the git hooks and do other sanity checks"
}

refresh_submodule_hooks()
{
local repo=$1
local hook
local hook_name

    if [ -d ${repo?}/.git ] ; then
        # use core's hook by default
	for hook_name in $(ls -1 "${COREDIR?}/.git-hooks") ; do
            hook="${repo?}/.git/hooks/${hook_name?}"
            if [ ! -e "${hook?}" -o -L "${hook?}" ] ; then
		rm -f "${hook?}"
		ln -sf "${COREDIR?}/.git-hooks/${hook_name?}" "${hook?}"
            fi
	done
        # override if need be by the submodules' own hooks
	for hook_name in $(ls -1 "${COREDIR?}/${repo?}/.git-hooks" 2>/dev/null) ; do
            hook="${repo?}/.git/hooks/${hook_name?}"
            if [ ! -e "${hook?}" -o -L "${hook?}" ] ; then
		rm -f "${hook?}"
		ln -sf "${COREDIR?}/${repo?}/.git-hooks/${hook_name?}" "${hook?}"
	    fi
	done
    elif [ -d .git/modules/${repo}/hooks ] ; then
	for hook_name in $(ls -1 "${COREDIR?}/.git-hooks") ; do
            hook=".git/modules/${repo?}/hooks/${hook_name?}"
            if [ ! -e "${hook?}" -o -L "${hook?}" ] ; then
		rm -f "${hook?}"
		ln -sf "${COREDIR?}/.git-hooks/${hook_name?}" "${hook?}"
            fi
	done
        # override if need be by the submodules' own hooks
	for hook_name in $(ls -1 "${COREDIR?}/${repo?}/.git-hooks" 2>/dev/null) ; do
            hook=".git/modules/${repo?}/hooks/${hook_name?}"
            if [ ! -e "${hook?}" -o -L "${hook?}" ] ; then
		rm -f "${hook?}"
		ln -sf "${COREDIR?}/${repo?}/.git-hooks/${hook_name?}" "${hook?}"
	    fi
	done
    fi

}

refresh_all_hooks()
{
local repo
local hook_name
local hook

    pushd "${COREDIR?}" > /dev/null
    for hook_name in $(ls -1 "${COREDIR?}/.git-hooks") ; do
        hook=".git/hooks/${hook_name?}"
        if [ ! -e "${hook?}" -o -L "${hook?}" ] ; then
            rm -f "${hook?}"
            ln -sf "${COREDIR?}/.git-hooks/${hook_name?}" "${hook?}"
        fi
    done

    for repo in ${SUBMODULES_ALL?} ; do
        refresh_submodule_hooks $repo
    done
    # In our workflow, it's always gerrit that does the submodule updates, so
    # better ignoring them to avoid accidentally including those changes in our
    # commits.
    # 'git submodule status' can be still used to see if a submodule has such
    # changes.
    for repo in ${SUBMODULES_CONFIGURED?} ; do
        git config submodule.$repo.ignore all
    done
    popd > /dev/null

}

set_push_url()
{
local repo

    repo="$1"
    if [ -n "$repo" ] ; then
	pushd "${COREDIR?}/${repo?}" > /dev/null
    else
	pushd "${COREDIR?}" > /dev/null
	repo="core"
    fi
    echo "setting up push url for ${repo?}"
    if [ "${repo?}" = "helpcontent2" ] ; then
	git config remote.origin.pushurl "ssh://${PUSH_USER}logerrit/help"
    else
	git config remote.origin.pushurl "ssh://${PUSH_USER}logerrit/${repo?}"
    fi
    popd > /dev/null
}

set_push_urls()
{
    PUSH_USER="$1"
    set_push_url
    for repo in ${SUBMODULES_ACTIVE?} ; do
	set_push_url "${repo?}"
    done
}

get_active_submodules()
{
SUBMODULES_ACTIVE=""
local repo

    for repo in ${SUBMODULES_ALL?} ; do
	if [ -d ${repo?}/.git -o -f ${repo?}/.git ] ; then
	    SUBMODULES_ACTIVE="${repo?} ${SUBMODULES_ACTIVE?}"
	fi
    done
}

get_configured_submodules()
{
    SUBMODULES_CONFIGURED=""
    if [ -f config_host.mk ] ; then
	SUBMODULES_CONFIGURED=$(cat config_host.mk | grep -a GIT_NEEDED_SUBMODULES | sed -e "s/.*=//")
    else
	# if we need the configured submoduel before the configuration is done. we assumed you want them all
	SUBMODULES_CONFIGURED=${SUBMODULES_ALL?}
    fi
}

get_git_reference()
{
    REFERENCED_GIT=""
    if [ -f config_host.mk ]; then
	REFERENCED_GIT=$(cat config_host.mk | grep -a GIT_REFERENCE_SRC | sed -e "s/.*=//")
    fi
    LINKED_GIT=""
    if [ -f config_host.mk ]; then
	LINKED_GIT=$(cat config_host.mk | grep -a GIT_LINK_SRC | sed -e "s/.*=//")
    fi
}

do_shortcut_update()
{
local module
local repo

    for module in $SUBMODULES_CONFIGURED ; do
	if [ ! -d ${module?}/.git ] ; then
	    case "${module?}" in
		helpcontent2)
		    if [ -d clone/help/.git ] ; then
			repo="clone/help/.git"
		    fi
		    ;;
		*)
		    if [ -d clone/${module?}/.git ] ; then
			repo="clone/${module?}/.git"
		    fi
		    ;;
	    esac
	    if [ -n "$repo" ] ; then
		cp -r "${repo?}" "${module?}/."
	    fi
	fi
    done
}

do_git_cmd()
{
    echo "cmd:$@"
    git "$@"
    git submodule foreach git "$@" $KEEP_GOING
}

do_checkout()
{
local cmd
local create_branch="0"
local branch
local module

    git checkout "$@" || return $?
    for cmd in "$@" ; do
	if [ "$cmd" = "-f" ]; then
	    return 0
	elif [ "$cmd" = "-b" ] ; then
	    create_branch=1
	elif [ "$create_branch" = "1" ] ; then
	    branch="$cmd"
	    create_branch=0
	fi
    done
    if [ -f .gitmodules ] ; then
	git submodule update
	if [ -n "$branch" ] ; then
	    git submodules foreach git checkout -b ${branch} HEAD || return $?
	fi
    else
	# now that is the nasty case we moved prior to submodules
	# delete the submodules left over if any
	for module in $SUBMODULES_ALL ; do
	    echo "clean-up submodule $module"
	    rm -fr ${module}
	done
	# make sure we have the needed repo in clone
	./g clone && ./g -f checkout "$@" || return $?
    fi
    return $?
}

do_reset()
{
    git reset "$@" || return $?
    if [ -f .gitmodules ] ; then
	git submodule update || return $?
    else
	# now that is the nasty case we moved prior to submodules
	# delete the submodules left over if any
	for module in $SUBMODULES_ALL ; do
	    echo "clean-up submodule $module"
	    rm -fr ${module}
	done
	# make sure we have the needed repo in clone
	./g clone && ./g -f reset "$@"
    fi
    return $?;
}

do_init_modules()
{
local module
local configured

    do_shortcut_update

    for module in $SUBMODULES_CONFIGURED ; do
	if [ -n "$LINKED_GIT" ] ; then
	    if ! [ -d ".git/modules/${module}" ]; then
		./bin/git-new-module-workdir "${LINKED_GIT}/${module}" "${module}"
	    fi
	fi
	configured=$(git config --local --get submodule.${module}.url)
	if [ -z "$configured" ] ; then
	    git submodule init $module || return $?
	fi
    done
    for module in $SUBMODULES_CONFIGURED ; do
        if [ -n "$REFERENCED_GIT" ] ; then
            git submodule update --reference $REFERENCED_GIT/.git/modules/$module $module || return $?
        else
            git submodule update $module || return $?
        fi
    done
    return 0
}


# no params, no action
if [ "$#" -eq "0" ] ; then
    usage
fi

if [ ! "`type -p git`" ]; then
    echo "Cannot find the git binary! Is git installed and is in PATH?"
    exit 1
fi


get_active_submodules
get_configured_submodules
get_git_reference




# extra params for some commands, like log
EXTRA=
COMMAND="$1"
PAGER=
RELATIVIZE=1
PUSH_ALL=
PUSH_USER=
PUSH_NOTES=
LAST_WORKING=
SET_LAST_WORKING=
ALLOW_EMPTY=
KEEP_GOING=
REPORT_REPOS=1
REPORT_COMMANDS=0
REPORT_COMPACT=0
DO_HOOK_REFRESH=false

while [ "${COMMAND:0:1}" = "-" ] ; do
    case "$COMMAND" in
        -f )KEEP_GOING="||:"
            ;;
        -z)
	    refresh_all_hooks
	    exit 0;
            ;;
	--set-push-urls)
	    shift
	    PUSH_USER="$1"
	    if [ -n "${PUSH_USER}" ] ; then
		PUSH_USER="${PUSH_USER}@"
	    fi
	    set_push_urls "$PUSH_USER"
	    exit 0;
	    ;;
	-*)
	    echo "option: $COMMAND not supported" 1>&2
	    exit 1
    esac
    shift
    COMMAND="$1"
done

shift

case "$COMMAND" in
    branch)
	do_git_cmd ${COMMAND} "$@"
	;;
    checkout)
	do_checkout "$@"
	;;
    clone)
	do_init_modules && refresh_all_hooks
        ;;
    fetch)
	(git fetch "$@" && git submodule foreach git fetch "$@" ) && git submodule update

	;;
    grep)
        KEEP_GOING="||:"
	do_git_cmd ${COMMAND} "$@"
	;;
    pull)
	git pull "$@" && git submodule update && refresh_all_hooks
	;;
    push)
	git submodule foreach git push "$@"
	if [ "$?" = "0" ] ; then
	    git push "$@"
	fi
	;;
    reset)
	do_reset
	;;
    tag)
	do_git_cmd ${COMMAND} "$@"
	;;
	"")
	;;
    *)
	echo "./g does not support command: $COMMAND" 1>&2
	exit 1;
        ;;
esac

exit $?

# vi:set shiftwidth=4 expandtab:
