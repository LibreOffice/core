#!/bin/sh

MYUID=`id | sed "s/(.*//g" | sed "s/.*=//"`

if [ $MYUID -ne 0 ]
then
    echo You need to have super-user rights to install this language package
    exit 1
fi

linenum=LINENUMBERPLACEHOLDER

more << "EOF"
LICENSEFILEPLACEHOLDER
EOF

agreed=
while [ x$agreed = x ]; do
    echo
    echo "Do you agree to the above license terms? [yes or no] "
    read reply leftover
    case $reply in
    y* | Y*)
        agreed=1;;
    n* | N*)
    echo "If you don't agree to the license you can't install this software";
    exit 1;;
    esac
done

echo
echo "Searching for the PRODUCTNAMEPLACEHOLDER installation ..."

# Determining current platform

platform=`uname -s`
case $platform in
SunOS)
  PACKAGENAME=`pkginfo -x | grep PRODUCTNAMEPLACEHOLDER-core | sed "s/ .*//"`
  PRODUCTINSTALLLOCATION="`pkginfo -r $PACKAGENAME`"
  ;;
Linux)
  RPMNAME=`rpm -qa | grep PRODUCTNAMEPLACEHOLDER-core`
  PRODUCTINSTALLLOCATION="`rpm -ql $RPMNAME | head -n 1`"
  ;;
*)
  echo "Unsupported platform"
  exit 1
  ;;
esac

# Asking for the installation directory

echo
echo "Where do you want to install the language pack ? [$PRODUCTINSTALLLOCATION] "
read reply leftover
if [ "x$reply" != "x" ]
then
  PRODUCTINSTALLLOCATION="$reply"
fi

# Unpacking

outdir=/var/tmp/install_$$
mkdir $outdir

#diskSpace=`df -k $outdir | tail -1 | awk '{if ( $4 ~ /%/) { print $3 } else { print $4 } }'`
#if [ $diskSpace -lt $diskSpaceRequired ]; then
#    printf "You will need atleast %s kBytes of Disk Free\n" $diskSpaceRequired
#    printf "Please free up the required Disk Space and try again\n"
#    exit 3
#fi

trap 'rm -rf $outdir; exit 1' HUP INT QUIT TERM
echo "Unpacking and installing..."

#if [ -x /usr/bin/sum ] ; then
#    echo "Checksumming..."
#
#    sum=`/usr/bin/sum $outdir/$outname`
#    index=1
#    for s in $sum
#    do
#   case $index in
#   1)  sum1=$s;
#     index=2;
#       ;;
#   2)  sum2=$s;
#       index=3;
#       ;;
#   esac
#    done
#    if expr $sum1 != <sum1replace> || expr $sum2 != <sum2replace> ; then
#   echo "The download file appears to be corrupted.  Please refer"
#   echo "to the Troubleshooting section of the Installation"
#   exit 1
#   fi
#else
#    echo "Can't find /usr/bin/sum to do checksum.  Continuing anyway."
#fi

case $platform in
SunOS)
  tail +$linenum $0 | gunzip | (cd $outdir; tar xvf -)
  adminfile=$outdir/admin.$$
  echo "basedir=$PRODUCTINSTALLLOCATION" > $adminfile
  /usr/sbin/pkgadd -d $outdir -a $adminfile PACKAGENAMEPLACEHOLDER

  ;;
Linux)
  tail +$linenum $0 > $outdir/PACKAGENAMEPLACEHOLDER
  rpm --prefix $PRODUCTINSTALLLOCATION -i $outdir/PACKAGENAMEPLACEHOLDER
  ;;
*)
  echo "Unsupported platform"
  exit 1
  ;;
esac

rm -rf $outdir

echo "Done..."

exit 0
