# Set the proper macros based on whether we are making the debugging version
# or not.  The valid parameters to this file are:
#
#	DEBUG=1		==> enable the making of the DBUG version
#	DBMALLOC=1	==> enable DBUG version with extensive MALLOC checking
#
#	DB_CFLAGS	==> CFLAGS is set to this value at the end if DEBUG=1
#	DB_LDFLAGS	==> LDFLAGS is set to this at the end if DEBUG=1
#	DB_LDLIBS       ==> LDLIBS is set to this at end if DEBUG=1
#
# The non debug versions of the above three macros are:
#
#	NDB_CFLAGS
#	NDB_LDFLAGS
#	NDB_LDLIBS
#
# One of the set of three should have values set appropriately prior to
# sourcing this file.

.IF $(DEBUG)
   DBUG_SRC	+= dbug.c
   DB_CFLAGS    += -Idbug/dbug

   .SETDIR=dbug/dbug : $(DBUG_SRC)

   # If DBMALLOC is requested (ie non-NULL) then include the sources for
   # compilation.  BSD 4.3 needs the getwd.c source compiled in due to a bug
   # in the clib getwd routine.
   .IF $(DBMALLOC)
      # Serious bug in bsd43 getwd.c would free a string and then use its
      # value.  The DBMALLOC code clears a string when it is free'd so the
      # value was no longer valid and the returned path for the current
      # directory was now completely wrong.
      .IF $(OSRELEASE) == bsd43
     GETWD_SRC += getwd.c
     .SETDIR=dbug : $(GETWD_SRC)
      .END

      MLC_SRC   += malloc.c free.c realloc.c calloc.c string.c\
           mlc_chk.c mlc_chn.c memory.c tostring.c m_perror.c\
           m_init.c mallopt.c dump.c

      .SETDIR=dbug/malloc : $(MLC_SRC)

      DB_CFLAGS += -Idbug/malloc
   .END

   SRC          += $(DBUG_SRC) $(MLC_SRC) $(GETWD_SRC)
   HDR		+= db.h 

   LDFLAGS	+= $(DB_LDFLAGS)
   LDLIBS	+= $(DB_LDLIBS)

   __.SILENT	!:= $(.SILENT)
   .SILENT	!:= yes
   TARGET	:= db$(TARGET)
   OBJDIR	:= $(OBJDIR).dbg
   .SILENT	!:= $(__.SILENT)

   CFLAGS	+= $(DB_CFLAGS)
   .KEEP_STATE  := _dbstate.mk
.ELSE
   CFLAGS       += $(NDB_CFLAGS)
   LDFLAGS      += $(NDB_LDFLAGS)
   LDLIBS       += $(NDB_LDLIBS)
.END
