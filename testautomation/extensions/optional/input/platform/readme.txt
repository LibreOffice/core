Extension which only support one platform
========================================================

freebsd_x86.oxt: freebsd_x86
freebsd_x86_86.oxt: freebsd_x86_64
linux_arm_eabi.oxt: linux_arm_eabi
linux_arm_oabi.oxt: linux_arm_oabi
linux_ia64.oxt: linux_ia64
linux_mips_eb.oxt: linux_mips_eb
linux_mips_el.oxt: linux_mips_el
linux_powerpc64.oxt: linux_powerpc64
linux_powerpc.oxt: linux_powerpc
linux_s390.oxt: linux_s390
linux_s390x.oxt: linux_s390x
linux_sparc.oxt: linux_sparc
linux_x86.oxt: linux_x86
linux_x86_64.oxt: linux_x86_64
macos_powerpc.oxt: macos_powerpc
macos_x86.oxt: macos_x86
solaris_sparc.oxt: solaris_sparc
solaris_x86.oxt: solaris_x86
windows_x86.oxt: windows_x86
os2_x86.oxt: os/2_x86

Extensions which support multiple platforms
=======================================================
mul1.oxt: windows_x86, linux_x86, solaris_x86


All platforms
=========================================================
all1.oxt: all, The <platform> element is missing. Default is "all".

all2.oxt: all, <platform value="all" />

all3.oxt: all, no description.xml



Invalid platforms
=========================================================
The following extensions cannot be installed because the platform element
is not correct. We assume that no valid platform is defined.

invalid1.oxt: <platform />

invalid2.oxt: <platform value=""/>

invalid3.oxt: <platform value="," />
