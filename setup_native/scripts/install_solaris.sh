#!/bin/bash

ADD="no"
LINK="no"
UPDATE="no"
USAGE="Usage: $0 [-a] [-l] [-h] <pkg-source-dir> <office-installation-dir>"

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
  echo "                            or language pack shell script containing the Solaris pkg packages"
  echo "    <office-installation-dir>: directory to where the office and the pkg database will get installed into"
  echo
  echo "Optional Parameter:"
  echo "    -a :              add to an existing <office-installation-dir>"
  echo "    -l :              create a link \"soffice\" in $HOME"
  echo "    -h :              output this help"
}

try_to_unpack_languagepack_file()
{
  FILENAME=$PACKAGE_PATH

  # Checking, if $FILENAME is a language pack.
  # String "language package" has to exist in the shell script file.
  # If this is no language pack, the installation is not supported

  SEARCHSTRING=`head -n 10 $FILENAME | grep "language package"`

  if [ ! -z "$SEARCHSTRING" ]
  then
    echo "First parameter $FILENAME is a language pack";
  else
    printf "\nERROR: First parameter $FILENAME is a file, but no language pack shell script.\n"
    echo $USAGE
    exit 2
  fi

  echo "Unpacking shell script $FILENAME"
  # TAILLINE=`head -n 20 $FILENAME | sed --quiet 's/linenum=//p'`
  TAILLINE=`head -n 20 $FILENAME | sed -n 's/linenum=//p'`

  if [ -x "/usr/bin/mktemp" ]  # available in Solaris 10
  then
    UNPACKDIR=`mktemp -d`
  else
    UNPACKDIR=/var/tmp/install_$$
    mkdir $UNPACKDIR
  fi

  echo $UNPACKDIR
  tail +$TAILLINE $FILENAME | gunzip | (cd $UNPACKDIR; tar xvf -)

  # Setting the new package path, in which the packages exist
  PACKAGE_PATH=$UNPACKDIR

  # Setting variable UPDATE, because an Office installation has to exist, if a language pack shall be installed
  UPDATE="yes"
}

pkg_error()
{
  # pkg command failed, check for admin log and report help
  if [ -f /tmp/.ai.pkg.zone.lock-afdb66cf-1dd1-11b2-a049-000d560ddc3e ]
  then
    echo "####################################################################"
    echo "#     Installation failed due to stale administrative lock         #"
    echo "####################################################################"
    printf "\nERROR: please remove the following file first:\n"
    ls -l /tmp/.ai.pkg.zone.lock-afdb66cf-1dd1-11b2-a049-000d560ddc3e
  fi
  rm -f $GETUID_SO
  exit 1
}

get_pkg_list()
{
  cd $1; ls -1
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

while getopts "alh" VALUE
do
  echo $VALUE
  case $VALUE in
    a)      ADD="yes"; break;;
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
  # If the first parameter is a shell script (download installation set), the packages have to
  # be unpacked into temp directory
  #
  if [ -f "$PACKAGE_PATH" ]
  then
    try_to_unpack_languagepack_file
  fi

  #
  # Create sed filter script for unwanted packages
  #

  cat > /tmp/userinstall_filer.$$ << EOF
/SUNWadabas/d
/^SUNWj[0-9]/d
/-desktop-int/d
/-shared-mime-info/d
/-cde/d
EOF

  # Do not install gnome-integration package on systems without GNOME
  pkginfo -q SUNWgnome-vfs
  if [ $? -ne 0 ]
  then

    echo '/-gnome/d' >> /tmp/userinstall_filer.$$
  fi

  # pkgdep sorts the packages based on their dependencies
  PKGDEP="`dirname $0`/pkgdep"
  if [ ! -x $PKGDEP ]; then
    PKGDEP="get_pkg_list"
  fi

  #
  # Get the list of packages to install
  #

  PKG_LIST=`$PKGDEP $PACKAGE_PATH | sed -f  /tmp/userinstall_filer.$$`
  rm -f /tmp/userinstall_filer.$$

  if [ -z "$PKG_LIST" ]
  then
    printf "\n$0: No packages found in $PACKAGE_PATH\n"
    exit 2
  fi

  echo "Packages found:"
  for i in $PKG_LIST ; do
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
  if [ "$ADD" = "no" ]
  then
    rmdir ${INSTALL_ROOT} 2>/dev/null
    if [ -d ${INSTALL_ROOT} ]
    then
      printf "\n$0: ${INSTALL_ROOT} exists and is not empty.\n"
      exit 2
    fi
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

if [ ! "${INSTALL_ROOT:0:1}" = "/" ]; then
  INSTALL_ROOT=`cd ${INSTALL_ROOT}; pwd`
fi

# This script must exist to make extension registration work
# always overwrite to get the latest version.
mkdir -p ${INSTALL_ROOT}/usr/lib
cat > ${INSTALL_ROOT}/usr/lib/postrun << \EOF
#!/bin/sh
set -e

if [ -x /usr/bin/mktemp ]
then
  CMD=`/usr/bin/mktemp /tmp/userinstall.XXXXXX`
else
  CMD=/tmp/userinstall.$$; echo "" > $CMD
fi

sed -e 's|/opt/|${PKG_INSTALL_ROOT}/opt/|g' > $CMD
/bin/sh -e $CMD
rm -f $CMD
EOF
chmod +x ${INSTALL_ROOT}/usr/lib/postrun 2>/dev/null

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
  # restore original "bootstraprc" prior to patching
  for i in ${PKG_LIST}; do
    my_dir=${INSTALL_ROOT}`pkgparam -d ${PACKAGE_PATH} "$i" BASEDIR`
    find "$my_dir" -type f -name bootstraprc.orig -exec sh -ce 'mv "$0" `dirname "$0"`/bootstraprc' {} \;
  done

  # copy INST_RELEASE file
  if [ ! -f ${INSTALL_ROOT}/var/sadm/system/admin/INST_RELEASE ]
  then
    mkdir -p ${INSTALL_ROOT}/var/sadm/system/admin 2>/dev/null
    cp -f /var/sadm/system/admin/INST_RELEASE ${INSTALL_ROOT}/var/sadm/system/admin/INST_RELEASE
  fi

  # The case UPDATE="yes" is valid for patch installation and for language packs.
  # For patches the variable PKG_LIST is empty, for language packs it is not empty.
  # Patches have to be installed with patchadd, language packs with pkgadd

  if [ -z "${PKG_LIST}" ]
  then
    LD_PRELOAD_32=$GETUID_SO /usr/sbin/patchadd -R ${INSTALL_ROOT} -M ${PATCH_PATH} ${PATCH_LIST} 2>&1 | grep -v '/var/sadm/patch' || pkg_error
  else
    LD_PRELOAD_32=$GETUID_SO /usr/sbin/pkgadd -d ${PACKAGE_PATH} -R ${INSTALL_ROOT} ${PKG_LIST} >/dev/null || pkg_error
  fi

else

  # Create BASEDIR directories to avoid manual user interaction
  for i in ${PKG_LIST}; do
    mkdir -m 0755 -p ${INSTALL_ROOT}`pkgparam -d ${PACKAGE_PATH} $i BASEDIR` 2>/dev/null
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

  LD_PRELOAD_32=$GETUID_SO /usr/sbin/pkgadd -d ${PACKAGE_PATH} -R ${INSTALL_ROOT} ${PKG_LIST} >/dev/null || pkg_error
fi

rm -f $GETUID_SO
rm -f /tmp/.ai.pkg.zone.lock*

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
  rm -f $HOME/soffice 2>/dev/null
  find `cd "$INSTALL_ROOT" && pwd` -name soffice -type f -perm -u+x -exec /bin/sh -ce 'ln -sf "$0" "$HOME/soffice" && echo "Creating link from $0 to $HOME/soffice"' {} \;
fi

# patch the "bootstraprc" to create a self-containing installation
for i in ${PKG_LIST}; do
  my_dir=${INSTALL_ROOT}`pkgparam -d ${PACKAGE_PATH} "$i" BASEDIR`
  find "$my_dir" -type f -name bootstraprc -exec sh -ce 'test ! -f "$0".orig && mv "$0" "$0".orig && sed '\''s,^UserInstallation=$SYSUSERCONFIG.*,UserInstallation=$BRAND_BASE_DIR/../UserInstallation,'\'' "$0".orig > "$0"' {} \;
done

# if an unpack directory exists, it can be removed now
if [ ! -z "$UNPACKDIR" ]
then
  # for i in ${PKG_LIST}; do
  #   cd $UNPACKDIR; rm -rf $i
  # done
  # rmdir $UNPACKDIR

  rm -rf $UNPACKDIR
  echo "Removed temporary directory $UNPACKDIR"
fi

echo
echo "Installation done ..."
exit 0

