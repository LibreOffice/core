#!/bin/sh

linenum=LINENUMBERPLACEHOLDER

UNPACKDIR=/var/tmp/unpack_PRODUCTNAMEPLACEHOLDER

# Asking for the unpack directory

echo
echo "Where do you want to unpack the PRODUCTNAMEPLACEHOLDER download set ? [$UNPACKDIR] "
read reply leftover
if [ "x$reply" != "x" ]
then
  UNPACKDIR="$reply"
fi

# Unpacking

mkdir $UNPACKDIR

#diskSpace=`df -k $outdir | tail -1 | awk '{if ( $4 ~ /%/) { print $3 } else { print $4 } }'`
#if [ $diskSpace -lt $diskSpaceRequired ]; then
#    printf "You will need atleast %s kBytes of Disk Free\n" $diskSpaceRequired
#    printf "Please free up the required Disk Space and try again\n"
#    exit 3
#fi

trap 'rm -rf $UNPACKDIR; exit 1' HUP INT QUIT TERM
echo "Unpacking ..."

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

# Determining current platform

platform=`uname -s`

case $platform in
SunOS)
  tail +$linenum $0 | (cd $UNPACKDIR; tar xvf -)
  ;;
Linux)
  tail +$linenum $0 | (cd $UNPACKDIR; tar xvf -)
  ;;
*)
  echo "Unsupported platform"
  exit 1
  ;;
esac

echo "Successfully unpacked..."

echo "Trying to start setup..."

if [ -f $UNPACKDIR/setup ]
then
    chmod 775 $UNPACKDIR/setup
    $UNPACKDIR/setup
fi

echo "Done"

exit 0
