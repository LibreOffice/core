/*************************************************************************
 *
 *  $RCSfile: astoperation.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:30:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _IDLC_ASTOPERATION_HXX_
#include <idlc/astoperation.hxx>
#endif
#ifndef _IDLC_ASTTYPE_HXX_
#include <idlc/asttype.hxx>
#endif
#ifndef _IDLC_ASTBASETYPE_HXX_
#include <idlc/astbasetype.hxx>
#endif
#ifndef _IDLC_ASTPARAMETER_HXX_
#include <idlc/astparameter.hxx>
#endif
#ifndef _IDLC_ERRORHANDLER_HXX_
#include <idlc/errorhandler.hxx>
#endif

using namespace ::rtl;

void AstOperation::addExceptions(StringList* pExceptions)
{
    if ( isOneway() )
    {
        idlc()->error()->error1(EIDL_ONEWAY_RAISE_CONFLICT, this);
    }

    StringList::iterator iter = pExceptions->begin();
    StringList::iterator end = pExceptions->end();
    AstDeclaration* pDecl = NULL;
    while ( iter != end)
    {
        pDecl = lookupByName(*iter);
        if ( !pDecl )
        {
            idlc()->error()->lookupError(*iter);
            return;
        }
        if ( (pDecl->getNodeType() == NT_exception) )
        {
            m_exceptions.push_back(pDecl);
        } else
        {
            idlc()->error()->error1(EIDL_ILLEGAL_RAISES, this);
        }
        iter++;
    }
}

sal_Bool AstOperation::isVoid()
{
    if ( m_pReturnType && (m_pReturnType->getNodeType() == NT_predefined) )
    {
        if ( ((AstBaseType*)m_pReturnType)->getExprType() == ET_void )
            return sal_True;
    }
    return sal_False;
}

sal_Bool AstOperation::dumpBlob(RegistryTypeWriter& rBlob, sal_uInt16 index)
{
    sal_uInt16      nParam = getNodeCount(NT_parameter);
    sal_uInt16      nExcep = nExceptions();
    RTMethodMode    methodMode = RT_MODE_TWOWAY;

    if ( isOneway() )
        methodMode = RT_MODE_ONEWAY;

    rBlob.setMethodData(index, OStringToOUString(getLocalName(), RTL_TEXTENCODING_UTF8),
                        OStringToOUString(getReturnType()->getRelativName(), RTL_TEXTENCODING_UTF8),
                        methodMode, nParam, nExcep, getDocumentation());

    if ( nParam )
    {
        DeclList::iterator iter = getIteratorBegin();
        DeclList::iterator end = getIteratorEnd();
        AstDeclaration* pDecl = NULL;
        AstParameter* pParam = NULL;
        RTParamMode paramMode;
        sal_uInt16 paramIndex = 0;
        while ( iter != end )
        {
            pDecl = *iter;
            if ( pDecl->getNodeType() == NT_parameter )
            {
                AstParameter* pParam = (AstParameter*)pDecl;
                switch (pParam->getDirection())
                {
                    case DIR_IN :
                        paramMode = RT_PARAM_IN;
                        break;
                    case DIR_OUT :
                        paramMode = RT_PARAM_OUT;
                        break;
                    case DIR_INOUT :
                        paramMode = RT_PARAM_INOUT;
                        break;
                }

                rBlob.setParamData(index, paramIndex++,
                           OStringToOUString(pParam->getType()->getRelativName(), RTL_TEXTENCODING_UTF8),
                           OStringToOUString(pDecl->getLocalName(), RTL_TEXTENCODING_UTF8),
                           paramMode);
            }
            iter++;
        }
    }

    if ( nExcep )
    {
        DeclList::iterator iter = m_exceptions.begin();
        DeclList::iterator end = m_exceptions.end();
        sal_uInt16 exceptIndex = 0;
        while ( iter != end )
        {
            rBlob.setExcData(index, exceptIndex++,
                             OStringToOUString((*iter)->getRelativName(), RTL_TEXTENCODING_UTF8) );
            iter++;
        }
    }

    return sal_True;
}

AstDeclaration* AstOperation::addDeclaration(AstDeclaration* pDecl)
{
    if ( pDecl->getNodeType() == NT_parameter )
    {
        AstParameter* pParam = (AstParameter*)pDecl;
        if ( isOneway() &&
             (pParam->getDirection() == DIR_OUT || pParam->getDirection() == DIR_INOUT) )
        {
            idlc()->error()->error2(EIDL_ONEWAY_CONFLICT, pDecl, this);
            return NULL;
        }
    }
    return AstScope::addDeclaration(pDecl);
}
