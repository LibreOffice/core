#!/bin/sh

GNOMEXCMDS="/usr/bin/gksu"
GNOMECMDS="/usr/bin/gnome-terminal /opt/gnome/bin/gnome-terminal"
KDEXCMDS="/usr/bin/kdesu /opt/kde/bin/kdesu /opt/kde?/bin/kdesu"
KDECMDS="/usr/bin/konsole /opt/kde/bin/konsole /opt/kde?/bin/konsole"
MISCCMDS="/usr/bin/x-terminal-emulator /usr/X11/bin/xterm /usr/openwin/bin/xterm"

TITLE='${PRODUCTNAME} update'

#
# Usage
#

Usage () {
  if [ "`basename $0`" = "update" ]; then
    echo "Usage: `basename $0` [<path to ${PRODUCTNAME} installation>]"
  fi
  echo
  exit 2
}

make_tempfile() {

  # Always prefer mktemp when available
  if [ -x "$MKTEMP" ]; then
    "$MKTEMP" "/tmp/$1.XXXXXX"
  else
    TMPCMD="/tmp/$1.$$"
    touch $TMPCMD
    echo $TMPCMD
  fi

}

#
# make_command_file - generate a command file to pass to xterm & co
#
run_in_terminal () {

  TMPCMD=`make_tempfile 'OpenOffice.org-Online-Update'`

  cat >> $TMPCMD

  cat >> $TMPCMD << \EOF
if [ $? -eq 0 -a ! -z "$DISPLAY" ]; then
  echo
  echo "Press <Enter> to close this window"
  read DUMMY
fi
EOF

  cat >> $TMPCMD << EOF
rm -f $TMPCMD
EOF

  chmod a+x $TMPCMD
  exec "$@" -e $TMPCMD
}

#
# run the command line passed as positional parameters either via
# gksu/kdesu or via su command in a terminal emulator.
#
elevate() {

  TITLEOPT=""
  GEOMOPT=""
  case `basename $XTERM` in
    "xterm")
      TITLEOPT="-title"
      GEOMOPT="-geometry "
      ;;
    "gnome-terminal" | "konsole" | "x-terminal-emulator")
      TITLEOPT="--title"
      GEOMOPT="--geometry="
      ;;
  esac

  case `basename $SU` in
    "kdesu" )
      # --caption
      SUOPT="-t"
      ;;
    "gksu" )
      # --title
      SUOPT=""
      ;;
    "su" )
      SUOPT="- root -c"
      ;;
  esac

  ( echo "echo The application \"$@\" will update your ${PRODUCTNAME} installation."
    echo "echo"
    echo "$SU" "$SUOPT" "$@"
  ) | run_in_terminal "$XTERM" "$TITLEOPT" "$TITLE" ${GEOMOPT}+300+300
}


update_pkg() {
  ADMINFILE=`make_tempfile 'LibreOffice-Online-Update-admin'`

cat >> $ADMINFILE << EOF
action=nocheck
conflict=nocheck
setuid=nocheck
idepend=nocheck
rdepend=nocheck
mail=
EOF

  PKGLIST=""
  for i in `cd $1; ls -d *`; do
    pkginfo -q $i && PKGLIST="$PKGLIST $i"
  done

  pkgrm -n -a $ADMINFILE $PKGLIST
  pkgadd -n -a $ADMINFILE -d $1 $PKGLIST

  rm -f $ADMINFILE
}

#
# main
#

CMD="`basename $0`"
BASEDIR="`dirname $0`"
ABSBASEDIR="`cd $BASEDIR; pwd`"

if [ -z "$DISPLAY" ]; then
  SU="su"
  XTERM=""
else
  # define search order depending on the desktop in use ..
  if [ "`uname -s`" = "SunOS" ]; then
    XPROP=/usr/openwin/bin/xprop
    GREP=/usr/xpg4/bin/grep
    MKTEMP=/usr/bin/mktemp
  else
    if [ -x /usr/X11/bin/xprop ]; then
      XPROP=/usr/X11/bin/xprop
    else
      XPROP=/usr/bin/xprop
    fi
    GREP=grep
    MKTEMP=/bin/mktemp
  fi

  # use desktop specific tools where possible, but prefer X11 su over terminal emulators
  if $XPROP -root | $GREP -q '^KWIN_RUNNING'; then
    SULIST="$KDEXCMDS $GNOMEXCMDS"
    XTERMLIST="$KDECMDS $MISCCMDS"
  else
    SULIST="$GNOMEXCMDS $KDEXCMDS"
    XTERMLIST="$GNOMECMDS $MISCCMDS"
  fi

  # search for the first available terminal emulator
  for i in $XTERMLIST; do
    if [ -x $i ]; then
      XTERM=$i
      break
    fi
  done

  # we prefer gksu or kdesu if available
  SU="su"
  for i in $SULIST; do
    if [ -x $i ]; then
      SU=$i
      break
    fi
  done
fi

if [ "$CMD" = "update" ]; then
  if [ $# -gt 0 ]; then
    [ -d $1 -a $# -eq 1 ] || Usage
    elevate "$ABSBASEDIR/$CMD"
  else
    if [ -d "$BASEDIR/RPMS" ]; then
      # do not use --hash as the control sequence to edit the current line does not work
      # in our setup (at least not with kdesu)
      rpm -v --freshen `find "$BASEDIR"/RPMS -name '*.rpm'`
    elif [ -d "$BASEDIR/DEBS" ]; then
      dpkg --install --selected-only --recursive "$BASEDIR"/DEBS
    elif [ -d "$BASEDIR/packages" ]; then
      update_pkg "$BASEDIR/packages"
    fi
  fi
else
  Usage
fi
