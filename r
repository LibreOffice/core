#!/bin/sh

# Usage:
# main worklfow:
# git commit -am "really cool change"
# ./r
#
# With -n|--dry-run option in place, it will show you what it would do,
# without really doing it.
#
# With -v|-verbose option in place, be extra verbose.
#
# optionaly you can install this script in some directory 
# in your PATH, i. e.
# ln -s <path to you libo git repo dir>/r ~/bin/git-r
# 
# With it in place your workflow is now:
#
# git commit -am "really cool change"
# git r
# is that cool ot what ;-)
#
#
# TODO 1:
# make it working from different directory inside gir repo,
# no only root directory, i.e:
# cd pyuno/source/zipcore
# git r -v
# (with link to ./r it works already, but
# if would copy it to /usr/local/bin or some other locations
# it wouldn't work. it should guess the git repo and his main root,
# see git-bash.completion for sample.
#
#
# TODO 2:
# provide bash completion for it, type
# git r <place tab here> and it should show the sub commands
#
#
# TODO 3:
# still has to provide helper for squashing changes for subsequent
# commits:
#
# git commit -am "cool"
# git commit -am "even cooler"
# git commit -am "really cool"
# now one *must* do
# git rebase -i HEAD~n and squash it there.
# git r
# 
# how can we squish those commit in this script with i.e.:
# git r -squash
# will de the magic: # git rebase -i HEAD~n and squash it there.
# somehow in this script
# any ideas?
#

#GERRITHOST=gerrit.libreoffice.org
GERRITHOST=logerrit
GERRITURL=ssh://$GERRITHOST/core

get_SHA_for_change() {
	SHA=`ssh ${GERRITHOST?} gerrit query --all-approvals change:$1|grep ref|tail -1|cut -d: -f2`
}

ask_tristate() {
	case $1 in
		[yY] | [yY][Ee][Ss] | [+] )
			ANSWER=y
		;;
		[nN] | [n|N][O|o] | [-] )
			ANSWER=n
		;;
		[] )
			ANSWER=
		;;
		* )
			echo "Please answer with either +,-,y,n,yes,no or the empty string."
			exit 1
		;;
	esac

}

doit() {
	$dry && { echo "Dry run: $@"; } || "$@"
}

dry=false
verbose=false

while
case $1 in
    -n|--dry-run)
	dry=true
	;;
    -v|--verbose)
	verbose=true
	;;
    -nv|-vn)
	dry=true
	verbose=true
	;;
    *) break
	;;
esac
do
    test $# -gt 0 && shift;
done


case "$1" in
	h|help)
		echo "Usage: ./r [r-options] [subcommand] [subcommand-options]"
		echo "r-options:"
		echo "             -n|--dry-run          do not run the actual command, only show it"
		echo "             -v|--verbose          be extra verbose"
		echo ""
		echo "subcommands:"
		echo "             t|test                test your gerrit setup"
		echo "             n|nextchange [BRANCH] reset branch to the remote to start with the next change"
		echo "             co|checkout CHANGEID  checkout the changes for review"
		echo "             pull CHANGEID         pull (and merge) the changes on current branch"
		echo "             cherry-pick CHANGEID  cherry-pick the change on current branch"
		echo "             patch CHANGEID        show the change as a patch"
		echo "             query ....            query for changes for review on project core"
		echo "             cmd ....              <any other gerrit command>"
		echo "             review  [CHANGEID]    interactively review a change (current one if no changeid given)"
		echo "default:     ------ [BRANCH]       submit your change for review to a branch"
		echo ""
		echo "subcommand-options:"
		echo ""
		echo "             ------                see gerrit command help"

		exit
	;;
	t|test)
		cmd="ssh $GERRITHOST 2>&1|grep \"Welcome to Gerrit Code Review\""
		if $verbose
		then
			echo "$cmd"
		fi
		if test -n "`$cmd`"
		then
			echo "Your gerrit setup was succesfull!"
		else
			echo "There seems to be trouble."
			echo "please have the output of: ssh -vvvv logerrit"
			echo "at hand when looking for help."
		fi
	;;
	n|nextchange)
		CHANGEID=`git log --format=format:%b -1 HEAD|grep Change-Id|cut -d: -f2|tr -d \ `
		if test -z "$CHANGEID"
		then
			CHANGEID="NOCHANGEID"
		fi
		BACKUPBRANCH=backup/$CHANGEID-`date +%F-%H%M%S`
		git branch $BACKUPBRANCH
		if $verbose
		then
			echo "current state backed up as $BACKUPBRANCH"
		fi
		BRANCH=$2
		if test -z "$BRANCH"
		then
			BRANCH=`git symbolic-ref HEAD 2> /dev/null`
			BRANCH="${BRANCH##refs/heads/}"
			if test -z "$BRANCH"
			then
				echo "no branch specified, and could not guess the current branch"
				exit 1
			fi
			if $verbose
			then
				echo "no branch specified, guessing current branch $BRANCH"
			fi
		fi
		doit git reset --hard remotes/origin/$BRANCH
	;;
	co|checkout)
		get_SHA_for_change $2
		doit git fetch $GERRITURL $SHA && git checkout FETCH_HEAD
	;;
	pull)
		get_SHA_for_change $2
		doit git pull $GERRITURL $SHA
	;;
	cherry-pick)
		get_SHA_for_change $2
		doit git fetch $GERRITURL $SHA && git cherry-pick FETCH_HEAD
	;;
	patch)
		get_SHA_for_change $2
		doit git fetch $GERRITURL $SHA && git format-patch -1 --stdout FETCH_HEAD
	;;
	query)
		shift
		doit ssh ${GERRITHOST?} gerrit query project:core "$@"
	;;
	cmd)
		COMMAND=$2
		if test -z "$COMMAND"
		then
		    echo "please specifiy some gerrit command, sorry"
		    exit 1
		fi
		shift
		doit ssh ${GERRITHOST?} gerrit "$@"
	;;
	review)
		# dry-run not supported here, make sense to warn?
		CHANGEID=$2
		if test -z "$CHANGEID"
		then
			CHANGEID=`git log --format=format:%b -1 HEAD|grep Change-Id|cut -d: -f2`
			if test -z "$CHANGEID"
			then
				echo "could not find a Change-Id in your last commit, sorry"
				exit 1
			fi
			if $verbose
			then
				echo "no Change-Id given on the command line, reviewing change$CHANGEID"
			fi
		fi
		MESSAGEREQ=""
		read -p 'was the change verified to build sucessfully (+) or found not to build (-) or none of that ()? ' VERIFIED
		ask_tristate $VERIFIED
		case "$ANSWER" in
			"y")
				VERIFIEDFLAG=--verified=+1
			;;
			"n")
				VERIFIEDFLAG="--verified=-1"
				MESSAGEREQ="$MESSAGEREQ and explain why you could not verify this"
			;;
			*)
				VERIFIEDFLAG="--verified=0"
			;;

		esac
		read -p 'is the code looking good (+), bad (-) or none of that ()? ' CODEREVIEW
		ask_tristate $CODEREVIEW
		case "$ANSWER" in
			"y")
				read -p 'do you approve and submit the change (+) too, or prefer someone else to do that ()? ' CODEREVIEW
				ask_tristate $CODEREVIEW
				case "$ANSWER" in
					"y")
						CODEREVIEWFLAG="--codereview=2 --submit"
					;;
					"n")
						CODEREVIEWFLAG="--codereview=1"
					;;
					*)
						CODEREVIEWFLAG="--codereview=1"
					;;
				esac
			;;
			"n")
				read -p 'do you still allow the change to go in () or not (-)? ' CODEREVIEW
				ask_tristate $CODEREVIEW
				case "$ANSWER" in
					"y")
						CODEREVIEWFLAG="--codereview=-1"
						MESSAGEREQ="$MESSAGEREQ and explain why you have reservations about the code"
					;;
					"n")
						CODEREVIEWFLAG="--codereview=-2"
						MESSAGEREQ="$MESSAGEREQ and explain why you want to block this"
					;;
					*)
						CODEREVIEWFLAG="--codereview=-1"
						MESSAGEREQ="$MESSAGEREQ and explain why you have reservations about the code"
					;;
				esac
			;;
			*)
			;;
		esac
		read -p "please type a friendly comment$MESSAGEREQ: " MESSAGE
		echo ssh ${GERRITHOST?} gerrit review -m \"$MESSAGE\" $VERIFIEDFLAG $CODEREVIEWFLAG $CHANGEID
	;;
	*)
		BRANCH=$2
		if test -z "$BRANCH"
		then
			BRANCH=`git symbolic-ref HEAD 2> /dev/null`
			BRANCH="${BRANCH##refs/heads/}"
			if test -z "$BRANCH"
			then
				echo "no branch specified, and could not guess the current branch"
				exit 1
			fi
			if $verbose
			then
				echo "no branch specified, guessing current branch $BRANCH"
			fi
		fi
		doit git push $GERRITURL HEAD:refs/for/$BRANCH
	;;

esac
