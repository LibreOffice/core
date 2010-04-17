#!/bin/sh

cd tools
../solenv/bin/build.pl -P4 -- -P4
../solenv/bin/deliver.pl
cd -
cd framework
../solenv/bin/build.pl -P4 -- -P4
../solenv/bin/deliver.pl
cd -
cd toolkit
../solenv/bin/build.pl -P4 -- -P4
../solenv/bin/deliver.pl
cd -
cd sw
../solenv/bin/build.pl -P4 -- -P4
../solenv/bin/deliver.pl
cd -
