#!/bin/bash

LINK="no"
UPDATE="no"
USAGE="Usage: $0 [-l] [-h] <pkg-source-dir> <office-installation-dir>"

help()
{
  echo
  echo "User Mode Installation script for developer and knowledgeable early access tester"
  echo
  echo "This installation method is not intended for use in a production environment!"
  echo "Using this script is unsupported and completely at your own risk"
  echo
  echo "Usage:" $0 "<pkg-source-dir> <office-installation-dir> [-l]"
  echo "    <pkg-source-dir>:       directory *only* containing the Solaris pkg packages to be installed"
  echo "    <office-installation-dir>: directory to where the office and the pkg database will get installed into"
  echo
  echo "Optional Parameter:"
  echo "    -l :              create a link \"soffice\" in $HOME"
  echo "    -h :              output this help"
}

#
# this script is for userland not for root
#

if [ $UID -eq 0 ]
then
  printf "\nThis script is for installation without administrative rights only\nPlease use pkgadd/patchadd to install as root\n"
  help
  exit 2
fi

while getopts "lh" VALUE
do
  echo $VALUE
  case $VALUE in
    h)      help; exit 0;;
    l)      LINK="yes"; break;;
    ?)      echo $USAGE; exit 2;;
  esac
done
shift `expr $OPTIND - 1`

if [ $# != 2 ]
then
  echo $USAGE
  exit 2
fi

# Determine whether this is a patch or a regular install set ..
/bin/bash -c "ls $1/*/patchinfo >/dev/null 2>&1"
if [ "$?" = 0 ]
then
  UPDATE="yes"
  PATCH_PATH="$1"
  PATCH_INFO_LIST=`/bin/bash -c "cd $1; ls */patchinfo"`
  PATCH_LIST=`for i in ${PATCH_INFO_LIST}; do dirname $i; done`
elif [ -f "$1/patchinfo" ]
then
  UPDATE="yes"
  PATCH_PATH=`dirname "$1"`
  PATCH_LIST=`basename "$1"`
else
  if [ -d "$1/packages" ]
  then
    PACKAGE_PATH="$1/packages"
  else
    PACKAGE_PATH=$1
  fi

  #
  # Check and get the list of packages to install
  #
  PKG_LIST=`cd $PACKAGE_PATH ; find * -type d -prune ! -name "*adabas*" \
    ! -name "*j5*" ! -name "*-desktop-int*" ! -name "*-shared-mime-info" \
    ! -name "*-cde*" ! -name "*-gnome*" -print`

  if [ -z "$PKG_LIST" ]
  then
    printf "\n$0: No packages found in $PACKAGE_PATH\n"
    exit 2
  fi

  # Do not install gnome-integration package on systems without GNOME
  pkginfo -q SUNWgnome-vfs
  if [ $? -eq 0 ]
  then
    GNOMEPKG=`cd $PACKAGE_PATH ; find * -type d -prune -name "*-gnome*" -print`
  fi

  echo "Packages found:"
  for i in $PKG_LIST $GNOMEPKG; do
    echo $i
  done
fi

INSTALL_ROOT=$2
if [ "$UPDATE" = "yes" ]
then
  if [ ! -d ${INSTALL_ROOT}/var/sadm/install/admin ]
  then
    printf "\n$0: No package database in ${INSTALL_ROOT}.\n"
    exit 2
  fi
else
  rmdir ${INSTALL_ROOT} 2>/dev/null
  if [ -d ${INSTALL_ROOT} ]
  then
    printf "\n$0: ${INSTALL_ROOT} exists and is not empty.\n"
    exit 2
  fi
  mkdir -p ${INSTALL_ROOT}/var/sadm/install/admin
fi

# Previous versions of this script did not write this file
if [ ! -f ${INSTALL_ROOT}/var/sadm/install/admin/default ]
then
  cat > ${INSTALL_ROOT}/var/sadm/install/admin/default << EOF
action=nocheck
conflict=nocheck
setuid=nocheck
idepend=nocheck
mail=
EOF
fi

# create local tmp directory to install on S10
LOCAL_TMP=
if [ -x /usr/bin/mktemp ]
then
  LOCAL_TMP=`mktemp -d`
  rmdir ${INSTALL_ROOT}/tmp 2>/dev/null
  ln -s ${LOCAL_TMP} ${INSTALL_ROOT}/tmp
fi

#
# the tail of the script contains a shared object for overloading the getuid()
# and a few other calls
#

GETUID_SO=/tmp/getuid.so.$$
linenum=???
tail +$linenum $0 > $GETUID_SO

#
# Perform the installation
#
if [ "$UPDATE" = "yes" ]
then
  if [ ! "${INSTALL_ROOT:0:1}" = "/" ]; then
    INSTALL_ROOT=`cd ${INSTALL_ROOT}; pwd`
  fi

  INSTALL_DIR=${INSTALL_ROOT}`pkgparam -f ${INSTALL_ROOT}/var/sadm/pkg/*core01/pkginfo BASEDIR`

  # restore original "bootstraprc" prior to patching
  mv -f ${INSTALL_DIR}/program/bootstraprc.orig ${INSTALL_DIR}/program/bootstraprc

  # copy INST_RELEASE file
  if [ ! -f ${INSTALL_ROOT}/var/sadm/system/admin/INST_RELEASE ]
  then
    mkdir -p ${INSTALL_ROOT}/var/sadm/system/admin 2>/dev/null
    cp -f /var/sadm/system/admin/INST_RELEASE ${INSTALL_ROOT}/var/sadm/system/admin/INST_RELEASE
  fi

  LD_PRELOAD_32=$GETUID_SO /usr/sbin/patchadd -R ${INSTALL_ROOT} -M ${PATCH_PATH} ${PATCH_LIST} 2>&1 | grep -v '/var/sadm/patch'
else

  # Create /usr directory required by co-packages like SUNWfreetype2
  mkdir -m 0755 -p ${INSTALL_ROOT}/usr

  #
  # Check/Create installation directory
  #

  for i in ${PKG_LIST}; do
    echo $i | grep core01 > /dev/null
    if [ $? = 0 ]; then
      INSTALL_DIR=${INSTALL_ROOT}`pkgparam -d ${PACKAGE_PATH} $i BASEDIR`
      mkdir -p ${INSTALL_DIR}
    fi
  done

  if [ ! "${INSTALL_ROOT:0:1}" = "/" ]; then
    INSTALL_ROOT=`cd ${INSTALL_ROOT}; pwd`
  fi

  echo "####################################################################"
  echo "#     Installation of the found packages                           #"
  echo "####################################################################"
  echo
  echo "Path to the packages       : " $PACKAGE_PATH
  echo "Path to the installation   : " $INSTALL_ROOT

  LD_PRELOAD_32=$GETUID_SO /usr/sbin/pkgadd -d ${PACKAGE_PATH} -R ${INSTALL_ROOT} ${PKG_LIST} ${GNOMEPKG} >/dev/null

  # Create symlinks in the program directory for all libraries installed to /usr
  for i in `cd ${INSTALL_ROOT}; find usr -name '*.so.*'`; do
    ln -s ../../../$i ${INSTALL_DIR}/program/`basename $i`
  done
fi

rm -f $GETUID_SO

# remove local tmp directory
if [ ! -z ${LOCAL_TMP} ]
then
  rm -f ${LOCAL_TMP}/.ai.pkg.zone.lock*
  rmdir ${LOCAL_TMP}
  rm -f ${INSTALL_ROOT}/tmp
  mkdir ${INSTALL_ROOT}/tmp
fi

#
# Create a link into the users home directory
#

if [ "$LINK" = "yes" ]
then
  echo
  echo "Creating link from $INSTALL_DIR/program/soffice to $HOME/soffice"
  rm -f $HOME/soffice 2>/dev/null
  ln -s $INSTALL_DIR/program/soffice $HOME/soffice
fi

# patch the "bootstraprc" to create a self-containing installation
mv ${INSTALL_DIR}/program/bootstraprc ${INSTALL_DIR}/program/bootstraprc.orig
sed 's/UserInstallation=$SYSUSERCONFIG\/.staroffice8/UserInstallation=$ORIGIN\/..\/UserInstallation/g' \
${INSTALL_DIR}/program/bootstraprc.orig > ${INSTALL_DIR}/program/bootstraprc

echo
echo "Installation done ..."
exit 0

