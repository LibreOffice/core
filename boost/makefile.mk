#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



# dmake create_clean -- just unpacks
# dmake patch -- unpacks and applies patch file
# dmake create_patch -- creates a patch file

PRJ=.

PRJNAME=boost
TARGET=ooo_boost

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# force patched boost for sunpro CC
# to workaround opt bug when compiling with -xO3
.IF "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")
all:
    @echo "An already available installation of boost should exist on your system."        
    @echo "Therefore the version provided here does not need to be built in addition."
.ELSE			# "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")

# --- Files --------------------------------------------------------

TARFILE_NAME=boost_1_48_0
TARFILE_MD5=d1e9a7a7f532bb031a3c175d86688d95
PATCH_FILES=\
    $(TARFILE_NAME).patch \
    boost_freebsd.patch

CONFIGURE_DIR=
CONFIGURE_ACTION=

BUILD_DIR=
BUILD_ACTION=
BUILD_FLAGS=

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

# --- post-build ---------------------------------------------------

# "normalize" the output structure, in that the C++ headers are
# copied to the canonic location in OUTPATH
# The allows, later on, to use the standard mechanisms to deliver those
# files, instead of delivering them out of OUTPATH/misc/build/..., which
# could cause problems

NORMALIZE_FLAG_FILE=so_normalized_$(TARGET)

$(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE) : $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
    -@$(MKDIRHIER) $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/*.h $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/*.hpp $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/accumulators $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/algorithm $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/archive $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/asio $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/assign $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/bimap $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/bind $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/chrono $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/circular_buffer $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/compatibility $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/concept $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/concept_check $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/config $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/date_time $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/detail $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/dynamic_bitset $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/exception $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/filesystem $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/flyweight $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/format $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/function $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/functional $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/function_types $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/fusion $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/geometry $(INCCOM)$/$(PRJNAME)   
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/gil $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/graph $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/graph $(INCCOM)$/$(PRJNAME)   
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/icl $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/integer $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/interprocess $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/intrusive $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/io $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/iostreams $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/iterator $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/lambda $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/logic $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/math $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/mpi $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/mpl $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/msm $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/multi_array $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/multi_index $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/numeric $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/optional $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/parameter $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/pending $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/phoenix $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/polygon $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/pool $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/preprocessor $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/program_options $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/property_map $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/property_tree $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/proto $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/ptr_container $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/random $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/range $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/ratio $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/regex $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/serialization $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/signals $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/signals2 $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/smart_ptr $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/spirit $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/statechart $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/system $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/test $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/thread $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/timer $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/tr1 $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/tuple $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/typeof $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/type_traits $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/units $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/unordered $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/utility $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/uuid $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/variant $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/wave $(INCCOM)$/$(PRJNAME)
    @$(GNUCOPY) -R $(PACKAGE_DIR)$/$(TARFILE_NAME)$/boost$/xpressive $(INCCOM)$/$(PRJNAME)
    @$(TOUCH) $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)

normalize: $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)

$(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) : normalize
.ENDIF			# "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")
