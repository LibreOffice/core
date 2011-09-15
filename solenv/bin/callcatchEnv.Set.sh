CC="callcatcher ${CC:-gcc}"
CXX="callcatcher ${CXX:-g++}"
AR="callarchive ${AR:-ar}"
#old-school ones, can go post-gbuildification is complete
LINK="callcatcher $CXX"
LIBMGR="callarchive ${LIBMGR:-ar}"
export CC CXX AR LINK LIBMGR
