/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_RESULT_HXX
#define SC_RESULT_HXX

#include <svl/svarray.hxx>
#include <vcl/timer.hxx>
#include <tools/string.hxx>


#include <com/sun/star/sheet/XVolatileResult.hpp>

#include <cppuhelper/implbase1.hxx> // helper for implementations


//class XResultListenerRef;
typedef ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >* XResultListenerPtr;
SV_DECL_PTRARR_DEL( XResultListenerArr_Impl, XResultListenerPtr, 4, 4 );


class ScAddInResult : public cppu::WeakImplHelper1<
                                com::sun::star::sheet::XVolatileResult>
{
private:
    String                  aArg;
    long                    nTickCount;
    XResultListenerArr_Impl aListeners;
    Timer                   aTimer;

    DECL_LINK( TimeoutHdl, Timer* );

    void                    NewValue();

public:
                            ScAddInResult(const String& rStr);
    virtual                 ~ScAddInResult();

//                          SMART_UNO_DECLARATION( ScAddInResult, UsrObject );

//  virtual BOOL            queryInterface( Uik, XInterfaceRef& );
//  virtual XIdlClassRef    getIdlClass(void);

                            // XVolatileResult
    virtual void SAL_CALL addResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
};


#endif

