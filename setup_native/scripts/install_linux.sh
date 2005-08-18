#!/bin/bash

LINK="no"
UPDATE="ask"
USAGE="Usage: $0 [-l,--link] [-U,--update] [-h,--help] <rpm-source-dir> <office-installation-dir>"

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
  echo "    <office-installation-dir>: directory to where the office will get installed into"
  echo
  echo "Optional Parameter:"
  echo "    -l,--link:              create a link \"soffice\" in $HOME"
  echo "    -U,--update:            update without asking"
  echo "    -h,--help:              output this help"
  echo
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

set -- `getopt -u -o 'lhU' -l 'link,help,update' -- $*`

if [ $? != 0 ]
then
  echo $USAGE
  exit 2
fi

for i in $*
do
  case $i in
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
# Check and get the list of packages to install
#

RPMLIST=`find $PACKAGE_PATH -maxdepth 2 -type f -name "*.rpm" ! -name "*-menus-*" ! -name "*-desktop-integration-*" ! -name "adabas*" ! -name "jre*" ! -name "*-gnome*" -print`

if [ -z "$RPMLIST" ]
then
  printf "\n$0: No packages found in $PACKAGE_PATH\n"
  exit 2
fi

# Do not install gnome-integration package on systems without GNOME
# check for /bin/rpm first, otherwise the rpm database is most likely
# empty anyway
if [ -x /bin/rpm ]
then
  GNOMERPM=`find $PACKAGE_PATH -type f -name "*.rpm" -name "*-gnome*" -print`
  /bin/rpm -q --whatprovides libgnomevfs-2.so.0 libgconf-2.so.4 >/dev/null || GNOMERPM=""
fi

#
# Determine whether this should be an update or a fresh install
#

INSTALLDIR=$2
RPM_DB_PATH=${INSTALLDIR}/.RPM_OFFICEDATABASE

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
}" ${INSTALLDIR}/program/bootstraprc 2>/dev/null | tr -d "\012"`

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
  mv -f ${INSTALLDIR}/program/bootstraprc.orig ${INSTALLDIR}/program/bootstraprc

  # the RPM_DB_PATH must be absolute
  if [ ! "${RPM_DB_PATH:0:1}" = "/" ]; then
    RPM_DB_PATH=`cd ${RPM_DB_PATH}; pwd`
  fi
else
  rmdir ${INSTALLDIR} 2>/dev/null

  if [ -d  ${INSTALLDIR} ]
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
  rpm --initdb --dbpath $RPM_DB_PATH
fi

echo "Packages found:"
for i in $RPMLIST $GNOMERPM; do
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
RELOCATIONS=`rpm -qp --qf "--relocate %{PREFIXES}=${NEWPREFIX} \n" $RPMLIST $GNOMERPM | sort -u | tr -d "\012"`
rpm --upgrade --nodeps --ignoresize -vh $RELOCATIONS --dbpath $RPM_DB_PATH $RPMLIST $GNOMERPM

#
# Create a link into the users home directory
#

if [ "$LINK" = "yes" ]
then
  echo
  echo "Creating link from ${INSTALLDIR}/program/soffice to $HOME/soffice"
  ln -sf $INSTALLDIR/program/soffice $HOME/soffice
fi

# patch the "bootstraprc" to create a self-containing installation
mv $INSTALLDIR/program/bootstraprc $INSTALLDIR/program/bootstraprc.orig
sed 's/UserInstallation=$SYSUSERCONFIG.*/UserInstallation=$ORIGIN\/..\/UserInstallation/g' $INSTALLDIR/program/bootstraprc.orig > $INSTALLDIR/program/bootstraprc

echo
echo "Installation done ..."

exit 0
