/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: result.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:17:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_RESULT_HXX
#define SC_RESULT_HXX

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif


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

