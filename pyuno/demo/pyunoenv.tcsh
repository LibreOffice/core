# the path to the office installation (e.g. /home/joe/OpenOffice.org1.1Beta)
setenv OOOHOME /src4/OpenOffice.org1.1Beta2

# don't modify anything beyond these lines
#---------------------------------------------
setenv PYTHONHOME $OOOHOME/program/python

if( ! $?LD_LIBRARY_PATH  ) then
    setenv LD_LIBRARY_PATH
endif

if(! $?PYTHONPATH ) then
    setenv PYTHONPATH
endif

if( ! $?LD_LIBRARY_PATH ) then
setenv LD_LIBRARY_PATH
endif

if ( $?PYTHOHOME )
	setenv PYTHONPATH $OOOHOME/program:$OOOHOME/program/pydemo:$OOOHOME/program/python/lib:$PYTHONPATH
else
	setenv PYTHONPATH $OOOHOME/program:$OOOHOME/program/pydemo:$OOOHOME/program/python/lib
endif
	
setenv LD_LIBRARY_PATH $OOOHOME/program:$LD_LIBRARY_PATH

if( $?PYTHONHOME ) then
setenv PATH $PYTHONHOME/bin:$PATH
endif
