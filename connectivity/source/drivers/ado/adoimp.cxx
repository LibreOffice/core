/*************************************************************************
 *
 *  $RCSfile: adoimp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2001-04-24 08:21:49 $
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

#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif

#ifndef _CONNECTIVITY_ADO_ADOIMP_HXX_
#include "ado/adoimp.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;


#define MYADOID(l) {l, 0,0x10,0x80,0,0,0xAA,0,0x6D,0x2E,0xA4};

const CLSID ADOS::CLSID_ADOCONNECTION_21    =   MYADOID(0x00000514);
const IID   ADOS::IID_ADOCONNECTION_21      =   MYADOID(0x00000550);

const CLSID ADOS::CLSID_ADOCOMMAND_21       =   MYADOID(0x00000507);
const IID   ADOS::IID_ADOCOMMAND_21         =   MYADOID(0x0000054E);

const CLSID ADOS::CLSID_ADORECORDSET_21     =   MYADOID(0x00000535);
const IID   ADOS::IID_ADORECORDSET_21       =   MYADOID(0x0000054F);

const CLSID ADOS::CLSID_ADOCATALOG_25       =   MYADOID(0x00000602);
const IID ADOS::IID_ADOCATALOG_25           =   MYADOID(0x00000603);

const CLSID ADOS::CLSID_ADOINDEX_25         =   MYADOID(0x0000061E);
const IID ADOS::IID_ADOINDEX_25             =   MYADOID(0x0000061F);

const CLSID ADOS::CLSID_ADOTABLE_25         =   MYADOID(0x00000609);
const IID ADOS::IID_ADOTABLE_25             =   MYADOID(0x00000610);

const CLSID ADOS::CLSID_ADOKEY_25           =   MYADOID(0x00000621);
const IID ADOS::IID_ADOKEY_25               =   MYADOID(0x00000622);

const CLSID ADOS::CLSID_ADOCOLUMN_25        =   MYADOID(0x0000061B);
const IID ADOS::IID_ADOCOLUMN_25            =   MYADOID(0x0000061C);

const CLSID ADOS::CLSID_ADOGROUP_25         =   MYADOID(0x00000615);
const IID ADOS::IID_ADOGROUP_25             =   MYADOID(0x00000616);

const CLSID ADOS::CLSID_ADOUSER_25          =   MYADOID(0x00000618);
const IID ADOS::IID_ADOUSER_25              =   MYADOID(0x00000619);

const CLSID ADOS::CLSID_ADOVIEW_25          =   MYADOID(0x00000612);
const IID ADOS::IID_ADOVIEW_25              =   MYADOID(0x00000613);

void ADOS::ThrowException(ADOConnection* _pAdoCon,const Reference< XInterface >& _xInterface) throw(SQLException, RuntimeException)
{
    ADOErrors *pErrors = NULL;
    _pAdoCon->get_Errors(&pErrors);
    if(!pErrors)
        return; // no error found

    pErrors->AddRef( );

    // alle aufgelaufenen Fehler auslesen und ausgeben
    sal_Int32 nLen;
    pErrors->get_Count(&nLen);
    if (nLen)
    {
        ::rtl::OUString sError;
        ::rtl::OUString aSQLState;
        SQLException aException;
        for (sal_Int32 i = nLen-1; i>=0; i--)
        {
            ADOError *pError = NULL;
            pErrors->get_Item(OLEVariant(i),&pError);
            WpADOError aErr(pError);
            OSL_ENSURE(pError,"No error in collection found! BAD!");
            if(pError)
            {
                if(i==nLen-1)
                    aException = SQLException(aErr.GetDescription(),_xInterface,aErr.GetSQLState(),aErr.GetNumber(),Any());
                else
                {
                    SQLException aTemp = SQLException(aErr.GetDescription(),
                        _xInterface,aErr.GetSQLState(),aErr.GetNumber(),makeAny(aException));
                    aTemp.NextException <<= aException;
                    aException = aTemp;
                }
            }
        }
        pErrors->Clear();
        pErrors->Release();
        throw aException;
    }
    pErrors->Release();
}

// -------------------------------------------------------------------------
sal_Int32 ADOS::MapADOType2Jdbc(DataTypeEnum eType)
{
    sal_Int32 nType;
    switch (eType)
    {
        case adUnsignedSmallInt:
        case adSmallInt:            nType = DataType::SMALLINT;     break;
        case adUnsignedInt:
        case adInteger:             nType = DataType::INTEGER;      break;
        case adUnsignedBigInt:
        case adBigInt:              nType = DataType::BIGINT;       break;
        case adSingle:              nType = DataType::FLOAT;        break;
        case adDouble:              nType = DataType::DOUBLE;       break;
        case adCurrency:            nType = DataType::DOUBLE;       break;
        case adVarNumeric:
        case adNumeric:             nType = DataType::NUMERIC;      break;
        case adDecimal:             nType = DataType::DECIMAL;      break;
        case adDate:
        case adDBDate:              nType = DataType::DATE;         break;
        case adDBTime:              nType = DataType::TIME;         break;
        case adDBTimeStamp:         nType = DataType::TIMESTAMP;    break;
        case adBoolean:             nType = DataType::BIT;          break;
//      case adArray:               nType = DataType::ARRAY;        break;
        case adBinary:
        case adGUID:                nType = DataType::BINARY;       break;
        case adBSTR:
        case adVarWChar:
        case adWChar:
        case adVarChar:             nType = DataType::VARCHAR;      break;
        case adLongVarWChar:
        case adLongVarChar:         nType = DataType::LONGVARCHAR;  break;
        case adVarBinary:           nType = DataType::VARBINARY;    break;
        case adLongVarBinary:       nType = DataType::LONGVARBINARY;break;
        case adChar:                nType = DataType::CHAR;         break;
        case adUnsignedTinyInt:
        case adTinyInt:             nType = DataType::TINYINT;      break;
        case adEmpty:               nType = DataType::SQLNULL;      break;
        case adUserDefined:
        case adPropVariant:
        case adFileTime:
        case adChapter:
        case adIDispatch:
        case adIUnknown:
        case adError:
        case adVariant:
                                    nType = DataType::OTHER;        break;
        default:
            OSL_ENSURE(0,"MapADOType2Jdbc: Unknown Type!");
            ;
    }
    return nType;
}
// -------------------------------------------------------------------------
DataTypeEnum ADOS::MapJdbc2ADOType(sal_Int32 _nType)
{
    switch (_nType)
    {
        case DataType::SMALLINT:        return adSmallInt;          break;
        case DataType::INTEGER:         return adInteger;           break;
        case DataType::BIGINT:          return adBigInt;            break;
        case DataType::FLOAT:           return adSingle;            break;
        case DataType::DOUBLE:          return adDouble;            break;
        case DataType::NUMERIC:         return adNumeric;           break;
        case DataType::DECIMAL:         return adDecimal;           break;
        case DataType::DATE:            return adDBDate;            break;
        case DataType::TIME:            return adDBTime;            break;
        case DataType::TIMESTAMP:       return adDBTimeStamp;       break;
        case DataType::BIT:             return adBoolean;           break;
        case DataType::BINARY:          return adBinary;            break;
        case DataType::VARCHAR:         return adVarWChar;          break;
        case DataType::LONGVARCHAR:     return adLongVarWChar;      break;
        case DataType::VARBINARY:       return adVarBinary;         break;
        case DataType::LONGVARBINARY:   return adLongVarBinary;     break;
        case DataType::CHAR:            return adWChar;             break;
        case DataType::TINYINT:         return adTinyInt;           break;
    default:
        OSL_ENSURE(0,"MapADOType2Jdbc: Unknown Type!");
            ;
    }
    return adEmpty;
}
// -----------------------------------------------------------------------------




