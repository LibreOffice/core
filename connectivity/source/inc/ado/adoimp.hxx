/*************************************************************************
 *
 *  $RCSfile: adoimp.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-30 13:20:58 $
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
#ifndef _CONNECTIVITY_ADO_ADOIMP_HXX_
#define _CONNECTIVITY_ADO_ADOIMP_HXX_

#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif

#ifndef _ADOCTINT_H_
#include <ado/ADOCTINT.H>
#endif

struct ADOConnection;
enum DataTypeEnum;
namespace connectivity
{
    namespace ado
    {

        class WpADOField;
        class ADOS
        {
        public:
            // Auch hier: BSTR mit SysFreeString() freigeben!
            inline static BSTR GetKeyStr()
            {
                return (BSTR)::rtl::OUString::createFromAscii("gxwaezucfyqpwjgqbcmtsncuhwsnyhiohwxz").getStr();
            }

            static const CLSID  CLSID_ADOCATALOG_25;
            static const IID    IID_ADOCATALOG_25;

            static const CLSID  CLSID_ADOCONNECTION_21;
            static const IID    IID_ADOCONNECTION_21;

            static const CLSID  CLSID_ADOCOMMAND_21;
            static const IID    IID_ADOCOMMAND_21;

            static const CLSID  CLSID_ADORECORDSET_21;
            static const IID    IID_ADORECORDSET_21;

            static const CLSID  CLSID_ADOINDEX_25;
            static const IID    IID_ADOINDEX_25;

            static const CLSID  CLSID_ADOCOLUMN_25;
            static const IID    IID_ADOCOLUMN_25;

            static const CLSID  CLSID_ADOKEY_25;
            static const IID    IID_ADOKEY_25;

            static const CLSID  CLSID_ADOTABLE_25;
            static const IID    IID_ADOTABLE_25;

            static const CLSID  CLSID_ADOGROUP_25;
            static const IID    IID_ADOGROUP_25;

            static const CLSID  CLSID_ADOUSER_25;
            static const IID    IID_ADOUSER_25;

            static const CLSID  CLSID_ADOVIEW_25;
            static const IID    IID_ADOVIEW_25;

            static void ThrowException(ADOConnection* _pAdoCon,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            static sal_Int32 MapADOType2Jdbc(DataTypeEnum eType);
            static DataTypeEnum MapJdbc2ADOType(sal_Int32 _nType,sal_Int32 _nJetEngine);
            static sal_Bool isJetEngine(sal_Int32 _nEngineType);

            static ObjectTypeEnum   mapObjectType2Ado(sal_Int32 objType);
            static sal_Int32        mapAdoType2Object(ObjectTypeEnum objType);
            static sal_Int32        mapAdoRights2Sdbc(RightsEnum eRights);
            static sal_Int32        mapRights2Ado(sal_Int32 nRights);

            static WpADOField       getField(ADORecordset* _pRecordSet,sal_Int32 _nColumnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };


    }
}

#define ADO_PROP(ItemName)                                          \
        WpADOProperty aProp(aProps.GetItem(ItemName));              \
        OLEVariant aVar;                                            \
        if(aProp.IsValid())                                         \
            aVar = aProp.GetValue();                                \
        else                                                        \
            ADOS::ThrowException(*m_pADOConnection,*this);


#endif //_CONNECTIVITY_ADO_ADOIMP_HXX_


