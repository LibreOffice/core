#!/bin/sh

MYUID=`id | sed "s/(.*//g" | sed "s/.*=//"`

if [ $MYUID -ne 0 ]
then
    echo You need to have super-user permissions to run this patch script
    exit 1
fi

echo
echo "Searching for the PRODUCTNAMEPLACEHOLDER installation ..."

RPMNAME=`rpm -qa | grep PRODUCTNAMEPLACEHOLDER-core01`

if [ "x$RPMNAME" != "x" ]
then
  PRODUCTINSTALLLOCATION="`rpm --query --queryformat "%{INSTALLPREFIX}" $RPMNAME`"
else
  echo "PRODUCTNAMEPLACEHOLDER is not installed"
  exit 1
fi

# Last chance to exit ..
echo
read -p "Patching the installation in ${PRODUCTINSTALLLOCATION}. Continue (y/n) ? " -n 1 reply leftover
echo
[ "$reply" == "y" ] || exit 1

echo
echo "About to update the following packages ..."

BASEDIR=`dirname $0`

RPMLIST=""
for i in `ls $BASEDIR/RPMS/*.rpm`
do
  rpm --query `rpm --query --queryformat "%{NAME}\n" --package $i` && RPMLIST="$RPMLIST $i"
done

# Save UserInstallation value
BOOTSTRAPRC="${PRODUCTINSTALLLOCATION}/program/bootstraprc"
USERINST=`grep UserInstallation ${BOOTSTRAPRC}`

# Check, if searchtoolbar extension rpm is available
SEARCHTOOLBARRPM=`ls $BASEDIR/RPMS/*.rpm | grep searchtoolbar`

if [ "x$SEARCHTOOLBARRPM" != "x" ]; then
  # Check, that $RPMLIST does not contain search toolbar rpm (then it is already installed)
  SEARCHTOOLBARINSTALLED=`grep searchtoolbar ${RPMLIST}`

  if [ "x$SEARCHTOOLBARINSTALLED" == "x" ]; then
    # Install the search toolbar rpm
    RPMLIST="$RPMLIST $SEARCHTOOLBARRPM"
  fi
fi

# Check, if kde-integration rpm is available
KDERPM=`ls $BASEDIR/RPMS/*.rpm | grep kde-integration`

if [ "x$KDERPM" != "x" ]; then
  # Check, that $RPMLIST does not contain kde integration rpm (then it is already installed)
  KDERPMINSTALLED=`grep kde-integration ${RPMLIST}`

  if [ "x$KDERPMINSTALLED" == "x" ]; then
    # Install the kde integration rpm
    RPMLIST="$RPMLIST $KDERPM"
  fi
fi

echo
rpm --upgrade -v --hash --prefix $PRODUCTINSTALLLOCATION --notriggers $RPMLIST
echo

# Check, if online update rpm is available
ONLINEUPDATERPM=`ls $BASEDIR/RPMS/*.rpm | grep onlineupdate`

if [ "x$ONLINEUPDATERPM" != "x" ]; then
  # Check, that $RPMLIST does not contain online update rpm (then it is already installed)
  ONLINEPDATEINSTALLED=`grep onlineupdate ${RPMLIST}`

  if [ "x$ONLINEPDATEINSTALLED" == "x" ]; then
    # Ask user, if online update shall be installed
    echo
    read -p "Do you want to install the new online update feature (y/n) ? " -n 1 reply leftover
    echo

    if [ "$reply" == "y" ]; then
      # Install the online update rpm
      rpm --install -v --hash --prefix $PRODUCTINSTALLLOCATION --notriggers $ONLINEUPDATERPM
    fi
  fi
fi

# Some RPM versions have problems with -U and --prefix
if [ ! -f ${BOOTSTRAPRC} ]; then
  echo Update failed due to a bug in RPM, uninstalling ..
  rpm --erase -v --nodeps --notriggers `rpm --query --queryformat "%{NAME} " --package $RPMLIST`
  echo
  echo Now re-installing new packages ..
  echo
  rpm --install -v --hash --prefix $PRODUCTINSTALLLOCATION --notriggers $RPMLIST
  echo
fi

# Restore the UserInstallation key if necessary
DEFUSERINST=`grep UserInstallation ${BOOTSTRAPRC}`
if [ "${USERINST}" != "${DEFUSERINST}" ]; then
  mv -f ${BOOTSTRAPRC} ${BOOTSTRAPRC}.$$
  sed "s|UserInstallation.*|${USERINST}|" ${BOOTSTRAPRC}.$$ > ${BOOTSTRAPRC}
  rm -f ${BOOTSTRAPRC}.$$
fi

echo "Done."

exit 0
