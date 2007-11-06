#!/bin/sh

jarfilename="JavaSetup.jar"
tempdir=/var/tmp/install_$$
java_runtime="java"
java_runtime_set="no"
java_runtime_found="no"
java_runtime_sufficient="no"
java_versions_supported="1.4 1.5 1.6"
rpm2cpio_found="no"
rpm_found="no"
sunjavahotspot="HotSpot"
errortext=""
errorcode=""

start_java()
{
    umask 022

    echo "Using $java_runtime"
    echo `$java_runtime -version`
    echo "Running installer"

    # looking for environment variables

    home=""
    if [ "x" != "x$HOME" ]; then
        home=-DHOME=$HOME
    fi

    log_module_states=""
    if [ "x" != "x$LOG_MODULE_STATES" ]; then
        log_module_states=-DLOG_MODULE_STATES=$LOG_MODULE_STATES
    fi

    getuid_path=""
    if [ "x" != "x$GETUID_PATH" ]; then
        getuid_path=-DGETUID_PATH=$GETUID_PATH
    fi

    if [ "x" != "x$jrefile" ]; then
        jrecopy=-DJRE_FILE=$jrefile
    fi

    # run the installer class file
    echo $java_runtime $home $log_module_states $getuid_path $jrecopy -jar $jarfilename
    $java_runtime $home $log_module_states $getuid_path $jrecopy -jar $jarfilename
}

cleanup()
{
    if [ "x$tempdir" != "x" -a -d "$tempdir" ]; then
        rm -rf $tempdir
    fi
}

do_exit()
{
    exitstring=$errortext
    if [ "x" != "x$errorcode" ]; then
        exitstring="$exitstring (exit code $errorcode)"
    fi

    # simply echo the exitstring or open a xterm
    # -> dependent from tty

    if tty ; then
        echo $exitstring
    else
        mkdir $tempdir

        # creating error file
        errorfile=$tempdir/error

        cat > $errorfile << EOF
echo "$exitstring"
echo "Press return to continue ..."
read a
EOF

        chmod 755 $errorfile

        # searching for xterm in path
        xtermname="xterm"
        xtermfound="no";
        for i in `echo $PATH | sed -e 's/^:/.:/g' -e 's/:$/:./g' -e 's/::/:.:/g' -e 's/:/ /g'`; do
            if [ -x "$i/$xtermname" -a ! -d "$i/$xtermname" ]; then
                xtermname="$i/$xtermname"
                xtermfound="yes"
                break
            fi
        done

        if [ $xtermfound = "no" -a "`uname -s`" = "SunOS" ]; then
            if [ -x /usr/openwin/bin/xterm ]; then
                xtermname=/usr/openwin/bin/xterm
                xtermfound="yes"
            fi
        fi

        if [ $xtermfound = "yes" ]; then
            $xtermname -e $errorfile
        fi
    fi

    cleanup

    exit $errorcode
}

set_jre_for_uninstall()
{
    # if "uninstalldata" exists, this is not required
    if [ ! -d "uninstalldata" ]; then
        packagepath="RPMS"
        jrefile=`find $packagepath -type f -name "jre*.rpm" -print`
        jrefile=`basename $jrefile`
        if [ -z "$jrefile" ]; then
            jrefile="notfound"
        fi

        # check existence of jre rpm
        if [ ! -f $packagepath/$jrefile ]; then
            errortext="Error: Java Runtime Environment (JRE) not found in directory: $packagepath"
            errorcode="4"
            do_exit
        fi
    fi
}

install_linux_rpm()
{
    # Linux requires usage of rpm2cpio to install JRE with user privileges
    # 1. --relocate /usr/java=/var/tmp does not work, because not all files are
    #    relocatable. Some are always installed into /etc
    # 2. --root only works with root privileges. With user privileges only the
    #    database is shifted, but not the files.

    # On Linux currently rpm2cpio is required (and rpm anyhow)

    find_rpm2cpio()

    if [ ! "$rpm2cpio_found" = "yes" ]; then
        errortext="Error: Did not find rpm2cpio. rpm2cpio is currently required for installations on Linux."
        errorcode="11"
        do_exit
    fi

    find_rpm()

    if [ ! "$rpm_found" = "yes" ]; then
        errortext="Error: Did not find rpm. rpm is currently required for installations on Linux."
        errorcode="12"
        do_exit
    fi

    # jrefile=jre-6-linux-i586.rpm
    # javahome=usr/java/jre1.6.0

    packagepath="RPMS"

    # using "uninstalldata" for uninstallation
    if [ -d "uninstalldata" ]; then
        packagepath="uninstalldata/jre"
    fi

    jrefile=`find $packagepath -type f -name "jre*.rpm" -print`
    jrefile=`basename $jrefile`
    if [ -z "$jrefile" ]; then
        jrefile="notfound"
    fi

    # check existence of jre rpm
    if [ ! -f $packagepath/$jrefile ]; then
        errortext="Error: Java Runtime Environment (JRE) not found in directory: $packagepath"
        errorcode="4"
        do_exit
    fi

    PACKED_JARS="lib/rt.jar lib/jsse.jar lib/charsets.jar  lib/ext/localedata.jar lib/plugin.jar lib/javaws.jar lib/deploy.jar"

    mkdir $tempdir

    trap 'rm -rf $tempdir; exit 1' HUP INT QUIT TERM

    tempjrefile=$tempdir/$jrefile
    cp $packagepath/$jrefile $tempjrefile

    if [ ! -f "$tempjrefile" ]; then
        errortext="Error: Failed to copy Java Runtime Environment (JRE) temporarily."
        errorcode="5"
        do_exit
    fi

    # check if copy was successful
    if [ -x /usr/bin/sum ]; then

        echo "Checksumming..."

        sumA=`/usr/bin/sum $packagepath/$jrefile`
        index=1
        for s in $sumA; do
            case $index in
                1)
                    sumA1=$s;
                    index=2;
                    ;;
                2)
                    sumA2=$s;
                    index=3;
                    ;;
            esac
        done

        sumB=`/usr/bin/sum $tempjrefile`
        index=1
        for s in $sumB; do
            case $index in
                1)
                    sumB1=$s;
                    index=2;
                    ;;
                2)
                    sumB2=$s;
                    index=3;
                    ;;
            esac
        done

        # echo "Checksum 1: A1: $sumA1 B1: $sumB1"
        # echo "Checksum 2: A2: $sumA2 B2: $sumB2"

        if [ $sumA1 -ne $sumB1 ] || [ $sumA2 -ne $sumB2 ]; then
            errortext="Error: Failed to install Java Runtime Environment (JRE) temporarily."
            errorcode="6"
            do_exit
        fi
    else
        echo "Can't find /usr/bin/sum to do checksum. Continuing anyway."
    fi

    # start to install jre
    echo "Extracting ..."
    olddir=`pwd`
    cd $tempdir
    rpm2cpio $tempjrefile | cpio -i --make-directories
    rm -f $tempjrefile # we do not need it anymore, so conserve discspace

    javahomeparent=usr/java
    javahomedir=`find $javahomeparent -maxdepth 1 -type d -name "jre*" -print`
    javahomedir=`basename $javahomedir`
    if [ -z "$javahomedir" ]; then
        javahomedir="notfound"
    fi

    javahome=$javahomeparent/$javahomedir

    if [ ! -d ${javahome} ]; then
        errortext="Error: Failed to extract the Java Runtime Environment (JRE) files."
        errorcode="7"
        do_exit
    fi

    UNPACK_EXE=$javahome/bin/unpack200
    if [ -f $UNPACK_EXE ]; then
        chmod +x $UNPACK_EXE
        packerror=""
        for i in $PACKED_JARS; do
            if [ -f $javahome/`dirname $i`/`basename $i .jar`.pack ]; then
                # printf "Creating %s\n" $javahome/$i
                $UNPACK_EXE $javahome/`dirname $i`/`basename $i .jar`.pack $javahome/$i
                if [ $? -ne 0 ] || [ ! -f $javahome/$i ]; then
                    printf "ERROR: Failed to unpack JAR file:\n\n\t%s\n\n" $i
                    printf "Installation failed. Please refer to the Troubleshooting Section of\n"
                    printf "the Installation Instructions on the download page.\n"
                    packerror="1"
                    break
                fi

                # remove the old pack file
                rm -f $javahome/`dirname $i`/`basename $i .jar`.pack
            fi
        done
        if [  "$packerror" = "1" ]; then
            if [ -d $javahome ]; then
                /bin/rm -rf $javahome
            fi

            errortext="Error: Failed to extract the Java Runtime Environment (JRE) files."
            errorcode="8"
            do_exit
        fi
    fi

    PREFS_LOCATION="`echo \"${javahome}\" | sed -e 's/^jdk.*/&\/jre/'`/.systemPrefs"

    if [ ! -d "${PREFS_LOCATION}" ]; then
        mkdir -m 755 "${PREFS_LOCATION}"
    fi
    if [ ! -f "${PREFS_LOCATION}/.system.lock" ]; then
        touch "${PREFS_LOCATION}/.system.lock"
        chmod 644 "${PREFS_LOCATION}/.system.lock"
    fi
    if [ ! -f "${PREFS_LOCATION}/.systemRootModFile" ]; then
        touch "${PREFS_LOCATION}/.systemRootModFile"
        chmod 644 "${PREFS_LOCATION}/.systemRootModFile"
    fi

    if [ x$ARCH = "x32" ] && [ -f "$javahome/bin/java" ]; then
        "$javahome/bin/java" -client -Xshare:dump > /dev/null 2>&1
    fi

    java_runtime=$tempdir/$javahome/bin/java

    # Make symbolic links to all TrueType font files installed in the system
    # to avoid garbles for Japanese
    language=`printenv LANG | cut -c 1-5`
    if [ x$language = "xja_JP" ]; then
        font_fallback_dir=$javahome/lib/fonts/fallback
        echo "Making symbolic links to TrueType font files into $font_fallback_dir."
        mkdir -p $font_fallback_dir
        ttf_files=`locate "*.ttf" | xargs`
        ln -s $ttf_files $font_fallback_dir
    fi

    echo "Done."
    cd $olddir
}

find_rpm2cpio()
{
    # searching for rpm2cpio in path
    for i in `echo $PATH | sed -e 's/^:/.:/g' -e 's/:$/:./g' -e 's/::/:.:/g' -e 's/:/ /g'`; do
        if [ -x "$i/rpm2cpio" -a ! -d "$i/$rpm2cpio" ]; then
            rpm2cpio_found="yes"
            break
        fi
    done
}

find_rpm()
{
    # searching for rpm in path
    for i in `echo $PATH | sed -e 's/^:/.:/g' -e 's/:$/:./g' -e 's/::/:.:/g' -e 's/:/ /g'`; do
        if [ -x "$i/rpm" -a ! -d "$i/$rpm" ]; then
            rpm_found="yes"
            break
        fi
    done
}

check_architecture()
{
    # Check, if system and installation set fit together (x86 and sparc).
    # If not, throw a warning.
    # Architecture of the installation set is saved in file "installdata/xpd/setup.xpd"
    # <architecture>sparc</architecture> or <architecture>i386</architecture>
    # Architecture of system is determined with "uname -p"

    setupxpdfile="installdata/xpd/setup.xpd"

    if [ -f $setupxpdfile ]; then
        platform=`uname -p` # valid values are "sparc" or "i386"
        searchstring="<architecture>$platform</architecture>"
        match=`cat $setupxpdfile | grep $searchstring`

        if [ -z "$match" ]; then
            # architecture does not fit, warning required
            if [ "$platform" = "sparc" ]; then
                echo "Warning: This is an attempt to install Solaris x86 packages on Solaris Sparc."
            else
                echo "Warning: This is an attempt to install Solaris Sparc packages on Solaris x86."
            fi
        fi
    fi
}

find_solaris_jre()
{
    # searching for java runtime in path
    for i in `echo $PATH | sed -e 's/^:/.:/g' -e 's/:$/:./g' -e 's/::/:.:/g' -e 's/:/ /g'`; do
        if [ -x "$i/$java_runtime" -a ! -d "$i/$java_runtime" ]; then
            java_runtime="$i/$java_runtime"
            java_runtime_found="yes"
            break
        fi
    done
}

check_jre_version()
{
    # check version of an installed JRE
    javaoutput=`$java_runtime -version 2>&1 | tail ${tail_args} -1`
    hotspot=`echo $javaoutput | grep $sunjavahotspot`
    if [ ! -z "$hotspot" ]; then
        for i in $java_versions_supported; do
            versionmatch=`echo $javaoutput | grep $i`
            if [ ! -z "$versionmatch" ]; then
                java_runtime_sufficient="yes"
                break
            fi
        done
    fi
}

# the user might want to specify java runtime on the commandline
USAGE="Usage: $0 [ -j <java_runtime> ]"
while getopts hj: opt; do
    echo "Parameter: $opt"
    case $opt in
        j)  java_runtime_set="yes";
            java_runtime="${OPTARG}"
            if [ ! -f "$java_runtime" ]; then
                errortext="Error: Invalid java runtime $java_runtime, file does not exist."
                errorcode="2"
                do_exit
            fi
            if [ ! -x "$java_runtime" ]; then
                errortext="Error: Invalid java runtime $java_runtime, not an executable file."
                errorcode="3"
                do_exit
            fi
            java_runtime_found="yes";
            ;;
        h)  echo ${USAGE}
            errortext=""
            errorcode=""
            do_exit
            ;;
        \?) echo ${USAGE}
            errortext=""
            errorcode=""
            do_exit
            ;;
    esac
done

# changing into setup directory
cd "`dirname "$0"`"

# prepare jre, if not set on command line
if [ "$java_runtime_set" != "yes" ]; then
    platform=`uname -s`
    if [ "`uname -s`" = "Linux" ]; then
        install_linux_rpm
    elif [ "`uname -s`" = "SunOS" ]; then
        check_architecture
        find_solaris_jre
        if [ "$java_runtime_found" = "yes" ]; then
            check_jre_version
            if [ ! "$java_runtime_sufficient" = "yes" ]; then
                errortext="Error: Did not find a valid Java Runtime Environment (JRE). Required JRE versions: $java_versions_supported"
                errorcode="9"
                do_exit
            fi
        else
            errortext="Error: Did not find an installed Java Runtime Environment (JRE)."
            errorcode="10"
            do_exit
        fi
    else
        errortext="Error: Platform $platform not supported for Java Runtime Environment (JRE) installation."
        errorcode="1"
        do_exit
    fi
fi

# jre for Linux is also required, if java runtime is set (for uninstallation mode)
if [ "$java_runtime_set" = "yes" ]; then
    platform=`uname -s`
    if [ "`uname -s`" = "Linux" ]; then
        set_jre_for_uninstall
    fi
fi

start_java

cleanup

exit 0
