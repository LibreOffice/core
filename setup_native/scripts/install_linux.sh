#!/bin/bash

ADD="no"
LINK="no"
UPDATE="ask"
UNPACKDIR=""
USAGE="Usage: $0 [-a,--add] [-l,--link] [-U,--update] [-h,--help] <rpm-source-dir> <office-installation-dir>"

help()
{
  echo
  echo "User Mode Installation script for developer and knowledgeable early access tester"
  echo
  echo "This installation method is not intended for use in a production environment!"
  echo "Using this script is unsupported and completely at your own risk"
  echo
  echo "Usage:" $0 [-lU] "<rpm-source-dir> <office-installation-dir>"
  echo "    <rpm-source-dir>: directory *only* containing the Linux rpm packages to be installed"
  echo "                      or language pack shell script containing the rpm packages"
  echo "    <office-installation-dir>: directory to where the office will get installed into"
  echo
  echo "Optional Parameter:"
  echo "    -a,--add:               add to an existing <office-installation-dir>"
  echo "    -l,--link:              create a link \"soffice\" in $HOME"
  echo "    -U,--update:            update without asking"
  echo "    -h,--help:              output this help"
  echo
}

try_to_unpack_languagepack_file()
{
  FILENAME=$PACKAGE_PATH

  # Checking, if $FILENAME is a language pack.
  # String "language package" has to exist in the shell script file.
  # If this is no language pack, the installation is not supported

  SEARCHSTRING=`head --lines=10 $FILENAME | grep "language package"`

  if [ ! -z "$SEARCHSTRING" ]
  then
    echo "First parameter $FILENAME is a language pack";
  else
    printf "\nERROR: First parameter $FILENAME is a file, but no language pack shell script.\n"
    echo $USAGE
    exit 2
  fi

  echo "Unpacking shell script $FILENAME"
  TAILLINE=`head --lines=20 $FILENAME | sed --quiet 's/linenum=//p'`

  UNPACKDIR=/var/tmp/install_$$
  mkdir $UNPACKDIR
  # UNPACKDIR=`mktemp -d`
  tail -n +$TAILLINE $FILENAME | gunzip | (cd $UNPACKDIR; tar xvf -)

  # Setting the new package path, in which the packages exist
  PACKAGE_PATH=$UNPACKDIR

  # Setting variable UPDATE, because an Office installation has to exist, if a language pack shall be installed
  UPDATE="yes"
}

#
# this script is for userland not for root
#

if [ $UID -eq 0 ]
then
  printf "\nThis script is for installation without administrative rights only\nPlease use rpm to install as root\n"
  help
  exit 2
fi

set -- `getopt -u -o 'alhU' -l 'add,link,help,update' -- $*`

if [ $? != 0 ]
then
  echo $USAGE
  exit 2
fi

for i in $*
do
  case $i in
    -a|--add)       ADD="yes"; shift;;
    -h|--help)      help; exit 0;;
    -l|--link)      LINK="yes"; shift;;
    -U|--update)    UPDATE="yes"; shift;;
    --)             shift; break;;
   esac
done

if [ $# != 2 ]
then
  echo $USAGE
  exit 2
fi

PACKAGE_PATH=$1

#
# If the first parameter is a shell script (download installation set), the packages have to
# be unpacked into temp directory
#

if [ -f "$PACKAGE_PATH" ]
then
  try_to_unpack_languagepack_file
fi

#
# Check and get the list of packages to install
#

RPMLIST=`find $PACKAGE_PATH -maxdepth 2 -type f -name "*.rpm" ! -name "*-menus-*" ! -name "*-desktop-integration-*" ! -name "adabas*" ! -name "jre*" -print`

if [ -z "$RPMLIST" ]
then
  printf "\n$0: No packages found in $PACKAGE_PATH\n"
  exit 2
fi

#
# Determine whether this should be an update or a fresh install
#

INSTALLDIR=$2

# Check for old style .RPM_OFFICEDATABASE first
if [ -d ${INSTALLDIR}/.RPM_OFFICEDATABASE ]; then
  RPM_DB_PATH=${INSTALLDIR}/.RPM_OFFICEDATABASE
else
  RPM_DB_PATH=${INSTALLDIR}/.RPM_DATABASE
fi

# Check for versionrc
if [ -f ${INSTALLDIR}/program/versionrc ]; then VERSIONRC=versionrc; fi

if [ "$UPDATE" = "ask" ]
then
  PRODUCT=`sed --silent -e "
/^buildid=/ {
s/buildid=\(.*\)/ [\1]/
h
}
/^ProductKey=/ {
s/ProductKey=//
G
p
}" ${INSTALLDIR}/program/${VERSIONRC:-bootstraprc} 2>/dev/null | tr -d "\012"`

  if [ ! -z "$PRODUCT" ]
  then
    echo
    echo "Found an installation of $PRODUCT in $INSTALLDIR"
    echo
    while [ "$UPDATE" != "yes" ]
    do
      read -a UPDATE -p "Do you want to update this installation (yes/no)? "
      if [ "$UPDATE" = "no" ]
      then
        exit 2
      fi
    done
  elif [ -d $RPM_DB_PATH -a "$ADD" = "no" ]
  then
    echo
    echo "The following packages are already installed in $INSTALLDIR"
    echo
    rpm --dbpath `cd $RPM_DB_PATH; pwd` --query --all
    echo
    while [ "$UPDATE" != "yes" ]
    do
      read -a UPDATE -p "Do you want to continue with this installation (yes/no)? "
      if [ "$UPDATE" = "no" ]
      then
        exit 2
      fi
    done
  else
    UPDATE="no"
  fi
fi

#
# Check/Create installation directory
#

if [ "$UPDATE" = "yes" ]
then
  # restore original bootstraprc
  mv -f ${INSTALLDIR}/program/bootstraprc.orig ${INSTALLDIR}/program/bootstraprc 2>/dev/null

  # the RPM_DB_PATH must be absolute
  if [ ! "${RPM_DB_PATH:0:1}" = "/" ]; then
    RPM_DB_PATH=`cd ${RPM_DB_PATH}; pwd`
  fi

  # we should use --freshen for updates to not add languages with patches, but this will break
  # language packs, so leave it for now ..
#  RPMCMD="--freshen"
  RPMCMD="--upgrade"
else
  rmdir ${INSTALLDIR} 2>/dev/null

  if [ -d  ${INSTALLDIR} -a "$ADD" = "no" ]
  then
    printf "\n$0: ${INSTALLDIR} exists and is not empty.\n"
    exit 2
  fi

  mkdir -p $RPM_DB_PATH || exit 2
  # XXX why ? XXX
  chmod 700 $RPM_DB_PATH

  # the RPM_DB_PATH must be absolute
  if [ ! "${RPM_DB_PATH:0:1}" = "/" ]; then
    RPM_DB_PATH=`cd ${RPM_DB_PATH}; pwd`
  fi

  # Creating RPM database and initializing
  if [ "$ADD" = "no" ]; then
    rpm --initdb --dbpath $RPM_DB_PATH
  fi

  # Default install command
  RPMCMD="--install"
fi

# populate the private rpm database with the dependencies needed
FAKEDBRPM=/tmp/fake-db-1.0-$$.noarch.rpm
linenum=???
tail -n +$linenum $0 > $FAKEDBRPM

rpm --upgrade --ignoresize --dbpath $RPM_DB_PATH $FAKEDBRPM

rm -f $FAKEDBRPM

echo "Packages found:"
for i in $RPMLIST ; do
  echo `basename $i`
done

#
# Perform the installation
#

echo
echo "####################################################################"
echo "#     Installation of the found packages                           #"
echo "####################################################################"
echo
echo "Path to the database:       " $RPM_DB_PATH
echo "Path to the packages:       " $PACKAGE_PATH
echo "Path to the installation:   " $INSTALLDIR
echo
echo "Installing the RPMs"

# inject a second slash to the last path segment to avoid rpm 3 concatination bug
NEWPREFIX=`cd ${INSTALLDIR}; pwd | sed -e 's|\(.*\)\/\(.*\)|\1\/\/\2|'`
RELOCATIONS=`rpm -qp --qf "--relocate %{PREFIXES}=${NEWPREFIX} \n" $RPMLIST | sort -u | tr -d "\012"`
UserInstallation=\$BRAND_BASE_DIR/../UserInstallation rpm $RPMCMD --ignoresize -vh $RELOCATIONS --dbpath $RPM_DB_PATH $RPMLIST

#
# Create a link into the users home directory
#

if [ "$LINK" = "yes" ]
then
  find `cd "$INSTALLDIR" && pwd` -name soffice -type f -perm /u+x -exec /bin/bash -ce 'ln -sf "$0" "$HOME/soffice" && echo "Creating link from $0 to $HOME/soffice"' {} \;
fi

if [ "$UPDATE" = "yes" -a ! -f $INSTALLDIR/program/bootstraprc ]
then
  echo
  echo "Update failed due to a bug in RPM, uninstalling .."
  rpm --erase -v --nodeps --dbpath $RPM_DB_PATH `rpm --query --queryformat "%{NAME} " --package $RPMLIST --dbpath $RPM_DB_PATH`
  echo
  echo "Now re-installing new packages .."
  echo
  rpm --install --nodeps --ignoresize -vh $RELOCATIONS --dbpath $RPM_DB_PATH $RPMLIST
  echo
fi

# patch the "bootstraprc" to create a self-containing installation
find "$INSTALLDIR" -type f -name bootstraprc -exec /bin/bash -ce 'test ! -e "$0".orig && mv "$0" "$0".orig && sed '\''s,^UserInstallation=$SYSUSERCONFIG.*,UserInstallation=$BRAND_BASE_DIR/../UserInstallation,'\'' "$0".orig > "$0"' {} \;

# if an unpack directory exists, it can be removed now
if [ ! -z "$UNPACKDIR" ]
then
  rm $UNPACKDIR/*.rpm
  rmdir $UNPACKDIR
  echo "Removed temporary directory $UNPACKDIR"
fi

echo
echo "Installation done ..."

exit 0
