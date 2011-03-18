/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include <osl/module.h>
#include <rtl/ustring.hxx>
#include <prewin.h>
#include <postwin.h>
#include "odma_lib.hxx"


namespace odma
{

    TODMRegisterApp         pODMRegisterApp;
    TODMUnRegisterApp       pODMUnRegisterApp;
    TODMSelectDoc           pODMSelectDoc;
    TODMOpenDoc             pODMOpenDoc;
    TODMSaveDoc             pODMSaveDoc;
    TODMCloseDoc            pODMCloseDoc;
    TODMNewDoc              pODMNewDoc;
    TODMSaveAs              pODMSaveAs;
    TODMActivate            pODMActivate;
    TODMGetDocInfo          pODMGetDocInfo;
    TODMSetDocInfo          pODMSetDocInfo;
    TODMGetDMSInfo          pODMGetDMSInfo;
    TODMGetDMSCount         pODMGetDMSCount;
    TODMGetDMSList          pODMGetDMSList;
    TODMGetDMS              pODMGetDMS;
    TODMSetDMS              pODMSetDMS;
    TODMQueryExecute        pODMQueryExecute;
    TODMQueryGetResults     pODMQueryGetResults;
    TODMQueryClose          pODMQueryClose;
    TODMCloseDocEx          pODMCloseDocEx;
    TODMSaveAsEx            pODMSaveAsEx;
    TODMSaveDocEx           pODMSaveDocEx;
    TODMSelectDocEx         pODMSelectDocEx;
    TODMQueryCapability     pODMQueryCapability;
    TODMSetDocEvent         pODMSetDocEvent;
    TODMGetAlternateContent pODMGetAlternateContent;
    TODMSetAlternateContent pODMSetAlternateContent;
    TODMGetDocRelation      pODMGetDocRelation;
    TODMSetDocRelation      pODMSetDocRelation;

    sal_Bool LoadFunctions(oslModule _pODMA);

    sal_Bool DMSsAvailable()
    {
        static sal_Bool bLoaded = sal_False;
        static sal_Bool bBeenHere = sal_False;
        oslModule pODMA = NULL;

        if (bBeenHere)
            return bLoaded;

        ::rtl::OUString sPath;
    #ifdef WNT
        sPath = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMA32.DLL"));
    #endif
    #ifdef UNX
        sPath = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("libodma.so"));
    #endif

        bBeenHere = sal_True;

        pODMA = osl_loadModule( sPath.pData,SAL_LOADMODULE_NOW );
        if( !pODMA)
            return sal_False;

        if (!LoadFunctions(pODMA))
            return sal_False;

        bLoaded = (NODMGetDMSCount() > 0);

        if (getenv ("NO_ODMA"))
            bLoaded = sal_False;

        return bLoaded;
    }
    // -------------------------------------------------------------------------

    sal_Bool LoadFunctions(oslModule pODMA)
    {
        if ( ( pODMRegisterApp = (TODMRegisterApp)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMRegisterApp")).pData)) == NULL ) return sal_False;
        if ( ( pODMUnRegisterApp = (TODMUnRegisterApp)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMUnRegisterApp")).pData)) == NULL ) return sal_False;
        if ( ( pODMSelectDoc = (TODMSelectDoc)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSelectDoc")).pData)) == NULL ) return sal_False;
        if ( ( pODMOpenDoc = (TODMOpenDoc)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMOpenDoc")).pData)) == NULL ) return sal_False;
        if ( ( pODMSaveDoc = (TODMSaveDoc)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSaveDoc")).pData)) == NULL ) return sal_False;
        if ( ( pODMCloseDoc = (TODMCloseDoc)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMCloseDoc")).pData)) == NULL ) return sal_False;
        if ( ( pODMNewDoc = (TODMNewDoc)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMNewDoc")).pData)) == NULL ) return sal_False;
        if ( ( pODMSaveAs = (TODMSaveAs)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSaveAs")).pData)) == NULL ) return sal_False;
        if ( ( pODMActivate = (TODMActivate)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMActivate")).pData)) == NULL ) return sal_False;
        if ( ( pODMGetDocInfo = (TODMGetDocInfo)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDocInfo")).pData)) == NULL ) return sal_False;
        if ( ( pODMSetDocInfo = (TODMSetDocInfo)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSetDocInfo")).pData)) == NULL ) return sal_False;
        if ( ( pODMGetDMSInfo = (TODMGetDMSInfo)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDMSInfo")).pData)) == NULL ) return sal_False;
        if ( ( pODMGetDMSCount = (TODMGetDMSCount)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDMSCount")).pData)) == NULL ) return sal_False;
        if ( ( pODMGetDMSList = (TODMGetDMSList)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDMSList")).pData)) == NULL ) return sal_False;
        if ( ( pODMGetDMS = (TODMGetDMS)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDMS")).pData)) == NULL ) return sal_False;
        if ( ( pODMSetDMS = (TODMSetDMS)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSetDMS")).pData)) == NULL ) return sal_False;
        if ( ( pODMQueryExecute = (TODMQueryExecute)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMQueryExecute")).pData)) == NULL ) return sal_False;
        if ( ( pODMQueryGetResults = (TODMQueryGetResults)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMQueryGetResults")).pData)) == NULL ) return sal_False;
        if ( ( pODMQueryClose = (TODMQueryClose)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMQueryClose")).pData)) == NULL ) return sal_False;
        if ( ( pODMCloseDocEx = (TODMCloseDocEx)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMCloseDocEx")).pData)) == NULL ) return sal_False;
        if ( ( pODMSaveAsEx = (TODMSaveAsEx)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSaveAsEx")).pData)) == NULL ) return sal_False;
        if ( ( pODMSaveDocEx = (TODMSaveDocEx)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSaveDocEx")).pData)) == NULL ) return sal_False;
        if ( ( pODMSelectDocEx = (TODMSelectDocEx)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSelectDocEx")).pData)) == NULL ) return sal_False;
        if ( ( pODMQueryCapability = (TODMQueryCapability)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMQueryCapability")).pData)) == NULL ) return sal_False;
        if ( ( pODMSetDocEvent = (TODMSetDocEvent)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSetDocEvent")).pData)) == NULL ) return sal_False;
        if ( ( pODMGetAlternateContent = (TODMGetAlternateContent)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetAlternateContent")).pData)) == NULL ) return sal_False;
        if ( ( pODMSetAlternateContent = (TODMSetAlternateContent)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSetAlternateContent")).pData)) == NULL ) return sal_False;
        if ( ( pODMGetDocRelation = (TODMGetDocRelation)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDocRelation")).pData)) == NULL ) return sal_False;
        if ( ( pODMSetDocRelation = (TODMSetDocRelation)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSetDocRelation")).pData)) == NULL ) return sal_False;
        return sal_True;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
