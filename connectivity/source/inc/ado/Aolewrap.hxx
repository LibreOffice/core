/*************************************************************************
 *
 *  $RCSfile: Aolewrap.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:25 $
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

namespace connectivity
{
    namespace ado
    {
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
        template<class T> class WpOLEBase
        {

        protected:
            T* pInterface;

        public:

            WpOLEBase();
            WpOLEBase(T* pInt);


            //inline
            WpOLEBase<T>& operator=(const WpOLEBase<T>& rhs)
            {
                if (rhs.pInterface != pInterface)
                {
                    if (pInterface) pInterface->Release();
                    pInterface = rhs.pInterface;
                    if (pInterface) pInterface->AddRef();
                }
                return *this;
            };

            WpOLEBase<T>& operator=(T* rhs)
            {
                if (pInterface != rhs)
                {
                    if (pInterface) pInterface->Release();
                    pInterface = rhs;
                    if (pInterface) pInterface->AddRef();
                }
                return *this;
            }

            WpOLEBase(const WpOLEBase<T>& aWrapper)
            {
                operator=(aWrapper);
            }

            virtual ~WpOLEBase()
            {
                if (pInterface)
                {
                    pInterface->Release();
                    pInterface = NULL;
                }
            }

            inline void clear()
            {
                if (pInterface)
                {
                    pInterface->Release();
                    pInterface = NULL;
                }
            }


            inline sal_Bool IsValid() const {return pInterface != NULL;};
            operator IDispatch*() { return pInterface; }
            operator T*() { return pInterface; }
        };


        template<class T> WpOLEBase<T>::WpOLEBase<T>()
        {
            pInterface = NULL;
        };


        template<class T> WpOLEBase<T>::WpOLEBase<T>(T* pInt)
        {
            pInterface = pInt;
        };
        // kein AddRef(), da
        // der nach Konvention von der erz. Methode
        // schon durchgeführt wurde.





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
        template<class Ts, class T, class WrapT> class WpOLECollection: public WpOLEBase<Ts>
        {
        public:
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpOLECollection(Ts* pInt):WpOLEBase<Ts>(pInt){}
            WpOLECollection(const WpOLECollection& rhs){operator=(rhs);}
            inline WpOLECollection& operator=(const WpOLECollection& rhs)
                {WpOLEBase<Ts>::operator=(rhs); return *this;};

            //////////////////////////////////////////////////////////////////////

            inline void Refresh(){pInterface->Refresh();};

            inline sal_Int32 GetItemCount() const
            {
                sal_Int32 nCount = 0;
                pInterface->get_Count(&nCount);
                return nCount;
            };

            inline WrapT GetItem(sal_Int32 index) const
            {
                OSL_ENSHURE(index >= 0 && index<GetItemCount(),"Wrong index for field!");
                T* pT;
                if (FAILED(pInterface->get_Item(OLEVariant(index), &pT))) return WrapT(NULL);
                return WrapT(pT);
            };

            inline WrapT GetItem(const ::rtl::OUString& sStr) const
            {
                T* pT;
                if (FAILED(pInterface->get_Item(OLEVariant(sStr), &pT))) return WrapT(NULL);
                return WrapT(pT);
            }
        };

        template<class Ts, class T, class WrapT> class WpOLEAppendCollection:
                public WpOLECollection<Ts,T,WrapT>
        {

        public:
            // Konstruktoren, operator=
            // diese rufen nur die Oberklasse
            WpOLEAppendCollection(Ts* pInt):WpOLECollection<Ts,T,WrapT>(pInt){}
            WpOLEAppendCollection(const WpOLEAppendCollection& rhs){operator=(rhs);}
            inline WpOLEAppendCollection& operator=(const WpOLEAppendCollection& rhs)
                {WpOLEBase<Ts>::operator=(rhs); return *this;};
            //////////////////////////////////////////////////////////////////////

            inline sal_Bool Append(WrapT aWrapT)
            {
                return SUCCEEDED(pInterface->Append(aWrapT.pInterface));
            };

            inline sal_Bool Delete(const ::rtl::OUString& sName)
            {
                return SUCCEEDED(pInterface->Delete(OLEVariant(sName)));
            };


        };
    }
}
#endif // _CONNECTIVITY_ADO_AOLEWRAP_HXX_

