/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XmlBuildList.cxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <EXTERN.h>               /* from the Perl distribution     */

#ifdef WNT
#include <tools/prewin.h>
#include "perl.h"
#include <tools/postwin.h>
#else
#include "perl.h"
#endif
#undef Copy                     /* from Perl handy.h */

#include <soldep/XmlBuildList.hxx>
#include "XmlBuildListDef.hxx"

#ifdef __cplusplus
#  define EXTERN_C extern "C"
#else
#  define EXTERN_C extern
#endif

static PerlInterpreter *my_perl;  /***    The Perl interpreter    ***/
static const char * DEP_MD_SIMPLE_STR = "md-simple";

EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);
static void xs_init (pTHX);
static void dl_init (pTHX);

static void xs_init(pTHX)
{
        char *file = __FILE__;
        dXSUB_SYS;
        {
        /* DynaLoader is a special case */
        newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
        }
}

static void dl_init(pTHX)
{
    char *file = __FILE__;
    dTARG;
    dSP;
/* Dynamicboot strapping code*/
        SAVETMPS;
        targ=sv_newmortal();
        FREETMPS;
/* end Dynamic bootstrapping code */
#ifdef MACOSX
    *file=0; // how does this works???
    sp=0;
#endif
}

FullByteStringList::FullByteStringList()
                    : FullByteStingListImpl(),
                    nRef (0)
{
}

FullByteStringList::~FullByteStringList()
{
    ByteString* pStr = First();
    while (pStr)
    {
        delete pStr;
        pStr = Next();
    }
}

ULONG FullByteStringList::GetPos (ByteString& rStr)
{
    ULONG nCurPos = GetCurPos();
    ULONG nPos = 0;
    ByteString* pStr = First();
    while (pStr)
    {
        if (*pStr == rStr)
        {
            GetObject (nCurPos);
            return nPos;
        }
        pStr = Next();
        nPos ++;
    }
    GetObject (nCurPos);
    return LIST_ENTRY_NOTFOUND;
}

FullByteStringListWrapper& FullByteStringListWrapper::operator=( const FullByteStringListWrapper& rFullByteStringListWrapper )
{
    if (pStringList)
        if (pStringList->GetRef() == 0)
            delete pStringList;
        else
            pStringList->DecRef();

    pStringList = rFullByteStringListWrapper.GetStringList();
    if (pStringList)
        pStringList->IncRef();
    return *this;
}

FullByteStringListWrapper::~FullByteStringListWrapper ()
{
    if (pStringList)
    {
        if (pStringList->GetRef() == 0)
            delete pStringList;
        else
            pStringList->DecRef();
    }
}

//
// Function initializes Perl
// ATTENTION: No check built in - YET TO DO
//
void XmlBuildList::initPerl(const char* ModulePath) {
    if (PerlInitialized)
        return;
    my_perl = perl_alloc();
    if (!my_perl)
        throw XmlBuildListException("Cannot initialize perl");
    perl_construct(my_perl);
    char* embedding[] = {"", "-e", "0"};
    int exitstatus = perl_parse(my_perl, xs_init, 3, embedding, (char **)NULL);
    if (!exitstatus) {
        dl_init(aTHX);
        exitstatus = perl_run(my_perl);
    }
    //perl_run(my_perl);
    // Create a variable (s. perlguts)
    SV* sv = get_sv("main::module_path", TRUE);
    sv_setpv(sv, ModulePath);
    eval_pv("use lib $main::module_path; use XMLBuildListParser;", FALSE);
    checkOperationSuccess();
    eval_pv("$main::build_lst_obj = XMLBuildListParser->new();", FALSE);
    checkOperationSuccess();
    eval_pv("@array = ();", FALSE);
    checkOperationSuccess();
    PerlInitialized = TRUE;
};

// Function proves if the $@ perl variable is set, if yes -
// last operation was unsuccessfull -> throws exception
void XmlBuildList::checkOperationSuccess() {
    char* op_result = SvPV_nolen(get_sv("main::@", FALSE));
    if (strcmp(op_result, ""))
        throw XmlBuildListException(op_result);
}

//
// Function generates a regular array with NULL as last element
// from the Perl-object @array
//
FullByteStringList* XmlBuildList::extractArray() {
    FullByteStringList* pStringList = new FullByteStringList();
    AV* theArrayObj = get_av("main::array", FALSE);
    I32 arrayLength = av_len(theArrayObj); // $#array value
    if (arrayLength == -1)
        return pStringList;

    SV** string_ptr;
    char* pStoredString;
    // populate vector with strings (char*)
    for (int i = 0; i <= arrayLength; i++) {
        string_ptr = av_fetch(theArrayObj, i, NULL);
        //pStoredString = savepv(SvPV_nolen(*string_ptr));
        pStoredString = SvPV_nolen(*string_ptr);
        ByteString* pStr = new ByteString(pStoredString);
        pStringList->Insert(pStr, LIST_APPEND);
    };
    return pStringList;
};


char* XmlBuildList::getError() {
    eval_pv("$main::string1 = $main::build_lst_obj->getError();", FALSE);
    checkOperationSuccess();
    return SvPV_nolen(get_sv("main::string1", FALSE));
};

/*****************************************************************************/
XmlBuildList::XmlBuildList(const ByteString& rModulePath)
/*****************************************************************************/
                : PerlInitialized (FALSE)
{
    initPerl(rModulePath.GetBuffer());
    string_obj1 = get_sv("main::string1", TRUE);
    string_obj2 = get_sv("main::string2", TRUE);
    string_obj3 = get_sv("main::string3", TRUE);
    if (!(string_obj1 && string_obj2 && string_obj3))
        throw XmlBuildListException("Cannot initialize Perl string objects");
};

//
// Function uninitializes Perl
//
/*****************************************************************************/
XmlBuildList::~XmlBuildList()
/*****************************************************************************/
{
    if (!PerlInitialized)
        return;
    perl_destruct(my_perl);
    perl_free(my_perl);
    PerlInitialized = FALSE;
};

/*****************************************************************************/
void XmlBuildList::loadXMLFile(const ByteString& rBuildList)
/*****************************************************************************/
{
    sv_setpv(string_obj1, rBuildList.GetBuffer());
    eval_pv("$main::string2 = $main::build_lst_obj->loadXMLFile($main::string1);", FALSE);
    checkOperationSuccess();
    if(!SvTRUE(string_obj2)) {
        const char* Message = getError();
        throw XmlBuildListException(Message);
    };
};

/*****************************************************************************/
FullByteStringListWrapper XmlBuildList::getProducts()
/*****************************************************************************/
{
    eval_pv("@array = $main::build_lst_obj->getProducts();", FALSE);
    checkOperationSuccess();
    FullByteStringList* pList = extractArray();
    return FullByteStringListWrapper(pList);
}

/*****************************************************************************/
FullByteStringListWrapper XmlBuildList::getJobDirectories(const ByteString& rJobType, const ByteString& rJobPlatform)
/*****************************************************************************/
{
    sv_setpv(string_obj1, rJobType.GetBuffer());
    sv_setpv(string_obj2, rJobPlatform.GetBuffer());
    eval_pv("@array = $main::build_lst_obj->getJobDirectories($main::string1, $main::string2);", FALSE);
    checkOperationSuccess();
    FullByteStringList* pList = extractArray();
    return FullByteStringListWrapper(pList);
}

/*****************************************************************************/
FullByteStringListWrapper XmlBuildList::getModuleDependencies(const ByteString& rProduct, const ByteString& rDependencyType)
/*****************************************************************************/
{
    FullByteStringList* pProducts = new FullByteStringList();
    FullByteStringListWrapper aProducts (pProducts);
    if (rProduct != "")
        aProducts.Insert (new ByteString(rProduct), LIST_APPEND);
    return getModuleDependencies(aProducts, rDependencyType);
};

/*****************************************************************************/
FullByteStringListWrapper XmlBuildList::getModuleDependencies(FullByteStringListWrapper& rProducts, const ByteString& rDependencyType)
/*****************************************************************************/
{
    eval_pv("@products = ();", FALSE);
    checkOperationSuccess();
    AV* theArrayObj = get_av("main::products", FALSE);
    FullByteStringList* pProducts = rProducts.GetStringList();
    ByteString* pStr = pProducts->First();
    while (pStr)
    {
        sv_setpv(string_obj2, pStr->GetBuffer());
        av_push(theArrayObj, string_obj2);
        pStr = pProducts->Next();
    }

    sv_setpv(string_obj1, rDependencyType.GetBuffer());
    eval_pv("@array = $main::build_lst_obj->getModuleDependencies(\\@products, $main::string1);", FALSE);
    checkOperationSuccess();
    FullByteStringList* pList = extractArray();
    return FullByteStringListWrapper(pList);
};

/*****************************************************************************/
FullByteStringListWrapper XmlBuildList::getJobBuildReqs(const ByteString& rJobDir, const ByteString& rBuildReqPlatform)
/*****************************************************************************/
{
    sv_setpv(string_obj1, rJobDir.GetBuffer());
    sv_setpv(string_obj2, rBuildReqPlatform.GetBuffer());
    eval_pv("@array = $main::build_lst_obj->getJobBuildReqs($main::string1, $main::string2);", FALSE);
    checkOperationSuccess();
    FullByteStringList* pList = extractArray();
    return FullByteStringListWrapper(pList);
}

/*****************************************************************************/
ByteString XmlBuildList::getModuleDepType(const ByteString& rDepModuleName)
/*****************************************************************************/
{
    sv_setpv(string_obj1, rDepModuleName.GetBuffer());
    eval_pv("$main::string1 = $main::build_lst_obj->getModuleDepType($main::string1);", FALSE);
    checkOperationSuccess();
    char* pString = SvPV_nolen(get_sv("main::string1", FALSE));
    ByteString sDependencyType(pString);
    return sDependencyType;
}

/*****************************************************************************/
sal_Bool XmlBuildList::hasModuleDepType(FullByteStringListWrapper& rProducts, const ByteString& rDependencyType)
/*****************************************************************************/
{
    FullByteStringListWrapper aDepencendModules = getModuleDependencies(rProducts, rDependencyType);
    if (aDepencendModules.Count()>0)
    {
        return sal_True;
    }
    return sal_False;
}

/*****************************************************************************/
FullByteStringListWrapper XmlBuildList::getModuleDepTypes(FullByteStringListWrapper& rProducts)
/*****************************************************************************/
{
    FullByteStringList * pList = new FullByteStringList();

    ByteString aDepType = ByteString( DEP_MD_SIMPLE_STR );
    bool bHasType = hasModuleDepType(rProducts, aDepType);
    if (bHasType)
        pList->Insert(new ByteString (aDepType));

    aDepType = ByteString(DEP_MD_ALWAYS_STR);
    bHasType = hasModuleDepType(rProducts, aDepType);
    if (bHasType)
        pList->Insert(new ByteString (aDepType));

    aDepType = ByteString(DEP_MD_FORCE_STR);
    bHasType = hasModuleDepType(rProducts, aDepType);
    if (bHasType)
        pList->Insert(new ByteString (aDepType));

    return FullByteStringListWrapper (pList);
}

/*****************************************************************************/
FullByteStringListWrapper XmlBuildList::getModuleProducts(const ByteString& rDepModuleName)
/*****************************************************************************/
{
    sv_setpv(string_obj1, rDepModuleName.GetBuffer());
    eval_pv("@array = $main::build_lst_obj->getModuleProducts($main::string1);", FALSE);
    checkOperationSuccess();
    FullByteStringList* pList = extractArray();
    return FullByteStringListWrapper(pList);
};

/*****************************************************************************/
ByteString XmlBuildList::getModuleName()
/*****************************************************************************/
{
    eval_pv("$main::string1 = $main::build_lst_obj->getModuleName();", FALSE);
    checkOperationSuccess();
    char* pString = SvPV_nolen(get_sv("main::string1", FALSE));
    ByteString sModuleName(pString);
    return sModuleName;
}

/*****************************************************************************/
FullByteStringListWrapper XmlBuildList::getDirDependencies(const ByteString& rJobDir, const ByteString& rJobType, const ByteString& rJobPlatform)
/*****************************************************************************/
{
    sv_setpv(string_obj1, rJobDir.GetBuffer());
    sv_setpv(string_obj2, rJobType.GetBuffer());
    sv_setpv(string_obj3, rJobPlatform.GetBuffer());
    eval_pv("@array = $main::build_lst_obj->getDirDependencies($main::string1, $main::string2, $main::string3);", FALSE);
    checkOperationSuccess();
    FullByteStringList* pList = extractArray();
    return FullByteStringListWrapper(pList);
};

/*****************************************************************************/
FullByteStringListWrapper XmlBuildList::getJobTypes(const ByteString& rJobDir)
/*****************************************************************************/
{
    sv_setpv(string_obj1, rJobDir.GetBuffer());
    eval_pv("@array = $main::build_lst_obj->getJobTypes($main::string1);", FALSE);
    checkOperationSuccess();
    FullByteStringList* pList = extractArray();
    return FullByteStringListWrapper(pList);
};

/*****************************************************************************/
FullByteStringListWrapper XmlBuildList::getJobPlatforms(const ByteString& rJobDir)
/*****************************************************************************/
{
    sv_setpv(string_obj1, rJobDir.GetBuffer());
    eval_pv("@array = $main::build_lst_obj->getJobPlatforms($main::string1);", FALSE);
    checkOperationSuccess();
    FullByteStringList* pList = extractArray();
    return FullByteStringListWrapper(pList);
};

/*****************************************************************************/
FullByteStringListWrapper XmlBuildList::getJobBuildReqPlatforms(const ByteString& rJobDir, const ByteString& rBuildReqName)
/*****************************************************************************/
{
    sv_setpv(string_obj1, rJobDir.GetBuffer());
    sv_setpv(string_obj2, rBuildReqName.GetBuffer());
    eval_pv("@array = $main::build_lst_obj->getJobBuildReqPlatforms($main::string1, $main::string2);", FALSE);
    checkOperationSuccess();
    FullByteStringList* pList = extractArray();
    return FullByteStringListWrapper(pList);
};
