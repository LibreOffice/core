When the executable is installed try to execute the executable "hello". The executable
file attribute (not on Windows) should be set.

CD into the extension directory in /user|share)/uno_packages/cache/uno_packages/xyz_
Then there are the directories for different platforms:

windows,
solaris,
linux

Each directory contains a hello executable. On linux one should execute it in a 
shell with an build environment, so that the c++ runtime is found.
