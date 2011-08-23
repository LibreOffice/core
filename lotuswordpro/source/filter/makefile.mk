PRJ=..$/..

PRJNAME=lotuswordpro
TARGET=filter
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

SLOFILES= \
    $(SLO)$/LotusWordProImportFilter.obj \
    $(SLO)$/genericfilter.obj       \
        $(SLO)$/lwpfilter.obj		\
        $(SLO)$/bencont.obj		\
        $(SLO)$/benlist.obj		\
        $(SLO)$/benname.obj		\
        $(SLO)$/benobj.obj		\
        $(SLO)$/benval.obj		\
        $(SLO)$/tocread.obj		\
        $(SLO)$/utbenvs.obj		\
        $(SLO)$/utlist.obj		\
        $(SLO)$/lwp9reader.obj		\
        $(SLO)$/lwpatomholder.obj	\
        $(SLO)$/lwpbasetype.obj		\
        $(SLO)$/lwpcolor.obj		\
        $(SLO)$/lwpcontent.obj		\
        $(SLO)$/lwpdefobj.obj		\
        $(SLO)$/lwpdlvlist.obj		\
        $(SLO)$/lwpdivinfo.obj		\
        $(SLO)$/lwpdivopts.obj		\
        $(SLO)$/lwpdoc.obj		\
        $(SLO)$/lwpfilehdr.obj		\
        $(SLO)$/lwpfont.obj		\
        $(SLO)$/lwpfoundry.obj		\
        $(SLO)$/lwpfrib.obj		\
        $(SLO)$/lwpfribtext.obj		\
        $(SLO)$/lwpfribframe.obj	\
        $(SLO)$/lwpfribbreaks.obj	\
        $(SLO)$/lwpfribsection.obj	\
        $(SLO)$/lwpfribptr.obj		\
        $(SLO)$/lwpfribtable.obj	\
        $(SLO)$/lwpholder.obj		\
        $(SLO)$/lwpidxmgr.obj		\
        $(SLO)$/lwplayout.obj		\
        $(SLO)$/lwplaypiece.obj		\
        $(SLO)$/lwplnopts.obj		\
        $(SLO)$/lwptablelayout.obj	\
        $(SLO)$/lwprowlayout.obj	\
        $(SLO)$/lwpcelllayout.obj	\
        $(SLO)$/lwpframelayout.obj	\
        $(SLO)$/lwppagelayout.obj	\
        $(SLO)$/lwpobj.obj		\
        $(SLO)$/lwpobjhdr.obj		\
        $(SLO)$/lwpobjid.obj		\
        $(SLO)$/lwpobjfactory.obj	\
        $(SLO)$/lwpobjstrm.obj		\
        $(SLO)$/lwppara.obj		\
        $(SLO)$/lwppara1.obj		\
        $(SLO)$/lwpparastyle.obj	\
        $(SLO)$/lwpprtinfo.obj		\
        $(SLO)$/lwpborderstuff.obj	\
        $(SLO)$/lwpparaborderoverride.obj	\
        $(SLO)$/lwpstory.obj		\
        $(SLO)$/lwpnumberingoverride.obj	\
        $(SLO)$/lwpcharborderoverride.obj	\
        $(SLO)$/lwpbreaksoverride.obj	\
        $(SLO)$/lwpbackgroundstuff.obj 	\
        $(SLO)$/lwptabrack.obj		\
        $(SLO)$/lwpcharacterstyle.obj	\
        $(SLO)$/lwpoverride.obj		\
        $(SLO)$/lwpsortopt.obj		\
        $(SLO)$/lwpsvstream.obj		\
        $(SLO)$/lwptoc.obj		\
        $(SLO)$/lwptools.obj		\
        $(SLO)$/lwpusrdicts.obj		\
        $(SLO)$/lwpuidoc.obj		\
        $(SLO)$/lwpdllist.obj       	\
        $(SLO)$/lwpparaproperty.obj 	\
        $(SLO)$/lwpsilverbullet.obj	\
        $(SLO)$/lwpcharsetmgr.obj   	\
        $(SLO)$/lwpsection.obj   	\
        $(SLO)$/lwppagehint.obj   	\
        $(SLO)$/lwpsdwfileloader.obj	\
        $(SLO)$/lwpoleobject.obj   	\
        $(SLO)$/lwpbulletstylemgr.obj   \
        $(SLO)$/lwpsdwgrouploaderv0102.obj   \
        $(SLO)$/lwpgrfobj.obj		\
        $(SLO)$/lwpdropcapmgr.obj	\
        $(SLO)$/lwptblcell.obj		\
        $(SLO)$/lwptable.obj		\
        $(SLO)$/lwpmarker.obj		\
        $(SLO)$/lwpproplist.obj 	\
        $(SLO)$/lwphyperlinkmgr.obj	\
        $(SLO)$/lwpvpointer.obj		\
        $(SLO)$/lwpdrawobj.obj	\
        $(SLO)$/lwpsdwrect.obj	\
        $(SLO)$/lwpfootnote.obj	\
        $(SLO)$/lwpfnlayout.obj	\
        $(SLO)$/lwpdocdata.obj	\
        $(SLO)$/lwpnotes.obj	\
        $(SLO)$/lwpverdocument.obj	\
        $(SLO)$/localtime.obj	\
        $(SLO)$/lwpfribmark.obj \
        $(SLO)$/lwpbookmarkmgr.obj \
        $(SLO)$/lwptblformula.obj\
        $(SLO)$/lwpnumericfmt.obj  \
        $(SLO)$/lwpchangemgr.obj \
        $(SLO)$/explode.obj \
        $(SLO)$/lwpglobalmgr.obj

.INCLUDE :  target.mk
