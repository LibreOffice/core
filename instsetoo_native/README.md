native install-set creation

This is where you will find your natively packaged builds after the
build has completed. On windows these would live in:

workdir/*/installation/LibreOffice_Dev/native/install/en-US/*.msi

for example (nothing like a few long directory names before breakfast).

Also generates ini files for the instdir/ tree (which are unfortunately
duplicated for now between instsetoo_native/CustomTarget_setup.mk and scp2).
