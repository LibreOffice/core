/*************************************************************************
 *
 *  $RCSfile: edfldexp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:18:47 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#pragma hdrstop

#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>        // GetCurFld
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>        // SwRefMarkFldUpdate
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace ::rtl;

/* -----------------28.11.2002 17:53-----------------
 *
 * --------------------------------------------------*/
BOOL SwEditShell::IsFieldDataSourceAvailable(String& rUsedDataSource) const
{
    const SwFldTypes * pFldTypes = GetDoc()->GetFldTypes();
    const USHORT nSize = pFldTypes->Count();
    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if( !xMgr.is() )
        return FALSE;
    Reference<XInterface> xInstance = xMgr->createInstance( OUString::createFromAscii( "com.sun.star.sdb.DatabaseContext" ));
    Reference<XNameAccess>  xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
    if(!xDBContext.is())
        return FALSE;
    for(USHORT i = 0; i < nSize; ++i)
    {
        SwFieldType& rFldType = *((*pFldTypes)[i]);
        USHORT nWhich = rFldType.Which();
        if(IsUsed(rFldType))
        {
            switch(nWhich)
            {
                case RES_DBFLD:
                {
                    SwClientIter aIter( rFldType );
                    SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
                    while(pFld)
                    {
                        if(pFld->IsFldInDoc())
                        {
                            const SwDBData& rData =
                                    ((SwDBFieldType*)pFld->GetFld()->GetTyp())->GetDBData();
                            try
                            {
                                return xDBContext->getByName(rData.sDataSource).hasValue();
                            }
                            catch(Exception)
                            {
                                rUsedDataSource = rData.sDataSource;
                                return FALSE;
                            }
                        }
                        pFld = (SwFmtFld*)aIter.Next();
                    }
                }
                break;
            }
        }
    }
    return TRUE;
}


