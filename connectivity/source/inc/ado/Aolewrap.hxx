/*************************************************************************
 *
 *  $RCSfile: Aolewrap.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:39:33 $
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
#ifndef _CONNECTIVITY_ADO_AOLEWRAP_HXX_
#define _CONNECTIVITY_ADO_AOLEWRAP_HXX_

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#include <map>
#include <vector>

#ifndef CONNECTIVITY_STDTYPEDEFS_HXX
#include "connectivity/StdTypeDefs.hxx"
#endif

namespace rtl
{
    class OUString;
}
namespace connectivity
{
    namespace ado
    {
        class OLEVariant;
        class WpBase
        {
        protected:
            IDispatch* pIUnknown;

            void setIDispatch(IDispatch* _pIUnknown);
        public:
            WpBase();
            WpBase(IDispatch* pInt);
            //inline
            WpBase& operator=(const WpBase& rhs);
            WpBase& operator=(IDispatch* rhs);
            WpBase(const WpBase& aWrapper);
            virtual ~WpBase();
            void clear();


            sal_Bool IsValid() const;
            operator IDispatch*();

        };
        //////////////////////////////////////////////////////////////////////////
        //
        // Template-Klasse WpOLEBase<class T>
        // ==================================
        //
        // Objekte dieser Klasse hält einen Zeiger auf ein Interface vom Typ T.
        // Es gibt Konstruktoren und Zuweisungsoperator die sicherstellen, dass
        // AddRef() und Release() entsprechend den COM-Konventionen gerufen werden.
        // Ein Objekt kann auch keinen Zeiger halten (Nullzeiger), dann ergibt
        // der Aufruf von IsValid() FALSE.
        //
        // Um effizientes pass-by-value machen zu koennen, ist diese (ebenso wie die
        // abgeleiteten Klassen) eine ganz schmale Wrapper-Klasse unter Vermeidung
        // virtueller Methoden und mit Inlining.

        //------------------------------------------------------------------------
        template<class T> class WpOLEBase : public WpBase
        {
        protected:
            T* pInterface;

        public:
            WpOLEBase(T* pInt = NULL) : WpBase(pInt),pInterface(pInt){}


            //inline
            WpOLEBase<T>& operator=(const WpOLEBase<T>& rhs)
            {
                WpBase::operator=(rhs);
                pInterface = rhs.pInterface;
                return *this;
            };

            WpOLEBase<T>& operator=(T* rhs)
            {
                WpBase::operator=(rhs);
                pInterface = rhs.pInterface;
                return *this;
            }

            WpOLEBase(const WpOLEBase<T>& aWrapper)
            {
                operator=(aWrapper);
            }

            virtual ~WpOLEBase()
            {
            }

            operator T*() const { return static_cast<T*>(pInterface); }
            void setWithOutAddRef(T* _pInterface)
            {
                pInterface = _pInterface;
                WpBase::setIDispatch(_pInterface);
            }
        };


        //////////////////////////////////////////////////////////////////////////
        //
        // Template-Klasse WpOLECollection<class Ts, class T, class WrapT>
        // ===============================================================
        //
        // Diese Klasse, welche sich von WpOLEBase<Ts> ableitet, abstrahiert die
        // den DAO-Collections gemeinsamen Eigenschaften:
        //
        // Sie werden über ein Interface Ts (etwa: DAOFields) angesprochen
        // und koennen ueber get_Item (hier:GetItem) Items des Typs T (genauer:
        // mit Interface T, etwa DAOField) herausgeben.
        //
        // Diese Wrapperklasse gibt aber nicht ein Interface T heraus,
        // sondern ein Objekt der Klasse WrapT. Dieses muß eine Konstruktion
        // durch T zulassen, vorzugsweise ist es von WpOLEBase<T> abgeleitet.
        //

        //------------------------------------------------------------------------
        template<class Ts, class T, class WrapT> class WpOLECollection : public WpOLEBase<Ts>
        {
        public:
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpOLECollection(Ts* pInt=NULL):WpOLEBase<Ts>(pInt){}
            WpOLECollection(const WpOLECollection& rhs){operator=(rhs);}
            inline WpOLECollection& operator=(const WpOLECollection& rhs)
                {WpOLEBase<Ts>::operator=(rhs); return *this;};

            //////////////////////////////////////////////////////////////////////

            inline void Refresh(){pInterface->Refresh();}

            inline sal_Int32 GetItemCount() const
            {
                sal_Int32 nCount = 0;
                return pInterface ? (SUCCEEDED(pInterface->get_Count(&nCount)) ? nCount : sal_Int32(0)) : sal_Int32(0);
            }

            inline WrapT GetItem(sal_Int32 index) const
            {
                OSL_ENSURE(index >= 0 && index<GetItemCount(),"Wrong index for field!");
                T* pT = NULL;
                WrapT aRet(NULL);
                if(SUCCEEDED(pInterface->get_Item(OLEVariant(index), &pT)))
                    aRet.setWithOutAddRef(pT);
                return aRet;
            }

            inline WrapT GetItem(const OLEVariant& index) const
            {
                T* pT = NULL;
                WrapT aRet(NULL);
                if(SUCCEEDED(pInterface->get_Item(index, &pT)))
                    aRet.setWithOutAddRef(pT);
                return aRet;
            }

            inline WrapT GetItem(const ::rtl::OUString& sStr) const
            {
                WrapT aRet(NULL);
                T* pT = NULL;
                if (FAILED(pInterface->get_Item(OLEVariant(sStr), &pT)))
                {
#if OSL_DEBUG_LEVEL > 0
                    ::rtl::OString sTemp("Unknown Item: ");
                    sTemp += ::rtl::OString(sStr.getStr(),sStr.getLength(),osl_getThreadTextEncoding());
                    OSL_ENSURE(0,sTemp);
#endif
                }
                else
                    aRet.setWithOutAddRef(pT);
                return aRet;
            }
            inline void fillElementNames(TStringVector& _rVector)
            {
                if(IsValid())
                {
                    Refresh();
                    sal_Int32 nCount = GetItemCount();
                    _rVector.reserve(nCount);
                    for(sal_Int32 i=0;i< nCount;++i)
                    {
                        WrapT aElement = GetItem(i);
                        if(aElement.IsValid())
                            _rVector.push_back(aElement.get_Name());
                    }
                }
            }
        };

        template<class Ts, class T, class WrapT> class WpOLEAppendCollection:
                public WpOLECollection<Ts,T,WrapT>
        {

        public:
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpOLEAppendCollection(Ts* pInt=NULL):WpOLECollection<Ts,T,WrapT>(pInt){}
            WpOLEAppendCollection(const WpOLEAppendCollection& rhs){ operator=(rhs); }
            inline WpOLEAppendCollection& operator=(const WpOLEAppendCollection& rhs)
                {WpOLEBase<Ts>::operator=(rhs); return *this;};
            //////////////////////////////////////////////////////////////////////

            inline sal_Bool Append(const WrapT& aWrapT)
            {
                return SUCCEEDED(pInterface->Append(OLEVariant((T*)aWrapT)));
            };

            inline sal_Bool Delete(const ::rtl::OUString& sName)
            {
                return SUCCEEDED(pInterface->Delete(OLEVariant(sName)));
            };


        };
    }
}
#endif // _CONNECTIVITY_ADO_AOLEWRAP_HXX_

