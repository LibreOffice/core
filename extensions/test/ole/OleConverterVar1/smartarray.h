/*************************************************************************
 *
 *  $RCSfile: smartarray.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:55 $
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
#ifndef _SMARTARRAY_H
#define _SMARTARRAY_H


template< class sourceType>
class SmartArray
{
    SAFEARRAY *m_array;
public:

    SmartArray( sourceType * parParams, int count, VARTYPE destVartype): m_array(NULL)
    {
        HRESULT hr= S_OK;
        SAFEARRAYBOUND rgsabound[1];
        rgsabound[0].cElements= count;
        rgsabound[0].lLbound= 0;
        m_array= SafeArrayCreate( destVartype, 1, rgsabound);
        SafeArrayLock( m_array);

        void* pData;
        if( m_array && (SUCCEEDED( SafeArrayAccessData( m_array, (void**)&pData)) ) )
        {

            for( int i=0; i< count; i++)
            {
                CComVariant varSource( parParams[i]);
                switch (destVartype)
                {
                case VT_I1:
                    {
                        char* p= (char*) pData;
                        if( SUCCEEDED( hr= varSource.ChangeType( destVartype)))
                            p[i]= V_I1( &varSource);
                        break;
                    }
                case VT_I2:
                    {
                        short* p= (short*) pData;
                        if( SUCCEEDED( hr=varSource.ChangeType( destVartype)))
                            p[i]= V_I2( &varSource);
                        break;
                    }
                case VT_UI2:
                    {
                        unsigned short* p= (unsigned short*) pData;
                        if( SUCCEEDED( hr=varSource.ChangeType( destVartype)))
                            p[i]= V_UI2( &varSource);
                        break;
                    }
                case VT_I4:
                    {
                        long* p= (long*)pData;
                    if( SUCCEEDED( hr=varSource.ChangeType( destVartype)))
                        p[i]= V_I4( &varSource);
                    break;
                    }
                case VT_UI4:
                    {
                        unsigned long* p= (unsigned long*)pData;
                        if( SUCCEEDED( hr=varSource.ChangeType( destVartype)))
                        p[i]= V_UI4( &varSource);
                        break;
                    }
                case VT_R4:
                    {
                        float* p= (float*)pData;
                    if( SUCCEEDED( hr=varSource.ChangeType( destVartype)))
                        p[i]= V_R4( &varSource);
                        break;
                    }
                case VT_R8:
                    {
                        double* p= (double*)pData;
                        if( SUCCEEDED( hr=varSource.ChangeType( destVartype)))
                        p[i]= V_R8( &varSource);
                        break;
                    }
                case VT_BOOL:
                    {
                        VARIANT_BOOL* p= (VARIANT_BOOL*)pData;
                        if( SUCCEEDED( hr=varSource.ChangeType( destVartype)))
                        p[i]= V_BOOL( &varSource);
                        break;
                    }
                case VT_BSTR:
                    {
                    BSTR* pBstr= ( BSTR*)pData;
                    if( SUCCEEDED( hr=varSource.ChangeType( destVartype)))
                        pBstr[i]= SysAllocString(V_BSTR( &varSource));
                    break;
                    }
                case VT_VARIANT:
                    {
                        VARIANT *pVariant= (VARIANT*)pData;
                        hr= VariantCopy( &pVariant[i], &varSource); break;
                    }
//              case VT_UNKNOWN:
//                  {
//                      long* pUnk= (long*)pData;
//                      pUnk[i]= reinterpret_cast<long>(parParams[i]);
//                      ((IUnknown*)pUnk[i])->AddRef(); break;
//                  }
//              case VT_DISPATCH:
//                  {
//                      long* pDisp= (long*)pData;
//                      pDisp[i]= (long)parParams[i];
//                      ((IDispatch*)pDisp[i])->AddRef(); break;
//                  }
                default:
                    hr= E_FAIL;
                }
            }
            if( FAILED( hr))
            {
                SafeArrayDestroy( m_array);
                m_array= NULL;
            }
        }
        SafeArrayUnaccessData( m_array);
    }
    ~SmartArray(){
        SafeArrayUnlock( m_array);
        SafeArrayDestroy( m_array );
    }

    operator bool (){ return m_array == NULL ?  false : true; }

    operator SAFEARRAY* (){ return m_array;}

};

template<>
class SmartArray<IUnknown*>
{
    SAFEARRAY *m_array;
public:

    SmartArray( sourceType * parParams, int count, VARTYPE destVartype);
//  {
//      ATLTRACE("SmartArray<IUnknown>");
//      HRESULT hr= S_OK;
//      SAFEARRAYBOUND rgsabound[1];
//      rgsabound[0].cElements= count;
//      rgsabound[0].lLbound= 0;
//      m_array= SafeArrayCreateVector( VT_UNKNOWN, 0, count);
//      SafeArrayLock( m_array);
//
//      IUnknown* *pData;
//      if( m_array && (SUCCEEDED( SafeArrayAccessData( m_array, (void**)&pData)) ) )
//      {
//
//          for( int i=0; i< count; i++)
//          {
//              CComVariant varSource( parParams[i]);
//              switch (destVartype)
//              {
//
//              case VT_UNKNOWN:
//                  {
//                      pData[i]= parParams[i];
//                      pData[i]->AddRef();
//                  }
//              default:
//                  hr= E_FAIL;
//              }
//          }
//          if( FAILED( hr))
//          {
//              SafeArrayDestroy( m_array);
//              m_array= NULL;
//          }
//      }
//      SafeArrayUnaccessData( m_array);
//  }
    ~SmartArray(){
        SafeArrayUnlock( m_array);
        SafeArrayDestroy( m_array );
    }

    operator bool (){ return m_array == NULL ?  false : true; }

    operator SAFEARRAY* (){ return m_array;}

};

template <> SmartArray <IUnknown*>::SmartArray(sourceType * parParams, int count, VARTYPE destVartype):m_array(NULL)
{
    ATLTRACE("SmartArray<IUnknown>");
    HRESULT hr= S_OK;
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].cElements= count;
    rgsabound[0].lLbound= 0;
    m_array= SafeArrayCreateVector( VT_UNKNOWN, 0, count);
    SafeArrayLock( m_array);

    IUnknown* *pData;
    if( m_array && (SUCCEEDED( SafeArrayAccessData( m_array, (void**)&pData)) ) )
    {
        for( int i=0; i< count; i++)
        {
            pData[i]= parParams[i];
            pData[i]->AddRef();
        }
    }
    SafeArrayUnaccessData( m_array);
};
#endif
