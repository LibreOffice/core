/*************************************************************************
 *
 *  $RCSfile: writerhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:39:48 $
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
 *  Contributor(s): cmc@openoffice.org
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
/// @HTML

#ifndef SW_WRITERHELPER
#define SW_WRITERHELPER

#include <typeinfo>
#include <vector>

#ifndef WW_TYPESSW
#include "typessw.hxx"
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>     //SfxItemPool
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>      //SfxItemSet
#endif
#ifndef _FORMAT_HXX
#include <format.hxx>               //SwFmt
#endif
#ifndef _NODE_HXX
#include <node.hxx>                 //SwCntntNode
#endif
#ifndef _DOC_HXX
#include <doc.hxx>                  //SwDoc
#endif
#ifndef _IPOBJ_HXX
#include <so3/ipobj.hxx>            //SvInPlaceObjectRef
#endif

class SwDoc;
class SwTxtFmtColl;
class SdrObject;
class SdrOle2Obj;
class SvPersist;

namespace sw
{
    /// STL container of Paragraph Styles (SwTxtFmtColl)
    typedef std::vector<SwTxtFmtColl *> ParaStyles;
    /// STL iterator for ParaStyles
    typedef std::vector<SwTxtFmtColl *>::iterator ParaStyleIter;
}

namespace sw
{
    namespace util
    {
        /** Provide a dynamic_cast style cast for SfxPoolItems

            A SfxPoolItem generally need to be cast back to its original type
            to be useful, which is both tedious and errorprone. So item_cast is
            a helper template to aid the process and test if the cast is
            correct.

            @param rItem
            The SfxPoolItem which is to be casted

            @tplparam T
            A SfxPoolItem derived class to cast rItem to

            @return A rItem upcasted back to a T

            @exception std::bad_cast Thrown if the rItem was not a T

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        template<class T> const T & item_cast(const SfxPoolItem &rItem)
            throw(std::bad_cast)
        {
            if (!rItem.IsA(STATICTYPE(T)))
                throw std::bad_cast();
            return static_cast<const T &>(rItem);
        }

        /** Provide a dynamic_cast style cast for SfxPoolItems

            A SfxPoolItem generally need to be cast back to its original type
            to be useful, which is both tedious and errorprone. So item_cast is
            a helper template to aid the process and test if the cast is
            correct.

            @param pItem
            The SfxPoolItem which is to be casted

            @tplparam T
            A SfxPoolItem derived class to cast pItem to

            @return A pItem upcasted back to a T or 0 if pItem was not a T

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        template<class T> const T * item_cast(const SfxPoolItem *pItem)
        {
            if (pItem && !pItem->IsA(STATICTYPE(T)))
                pItem = 0;
            return static_cast<const T *>(pItem);
        }

        /** Extract a SfxPoolItem derived property from a SwCntntNode

            Writer's attributes are retrieved by passing a numeric identifier
            and receiving a SfxPoolItem reference which must then typically be
            cast back to its original type which is both tedious and verbose.

            ItemGet uses item_cast () on the retrived reference to test that the
            retrived property is of the type that the developer thinks it is.

            @param rNode
            The SwCntntNode to retrieve the property from

            @param eType
            The numeric identifier of the property to be retrieved

            @tplparam T
            A SfxPoolItem derived class of the retrieved property

            @exception std::bad_cast Thrown if the property was not a T

            @return The T requested

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        template<class T> const T & ItemGet(const SwCntntNode &rNode,
            sal_uInt16 eType) throw(std::bad_cast)
        {
            return item_cast<T>(rNode.GetAttr(eType));
        }

        /** Extract a SfxPoolItem derived property from a SwFmt

            Writer's attributes are retrieved by passing a numeric identifier
            and receiving a SfxPoolItem reference which must then typically be
            cast back to its original type which is both tedious and verbose.

            ItemGet uses item_cast () on the retrived reference to test that the
            retrived property is of the type that the developer thinks it is.

            @param rFmt
            The SwFmt to retrieve the property from

            @param eType
            The numeric identifier of the property to be retrieved

            @tplparam T
            A SfxPoolItem derived class of the retrieved property

            @exception std::bad_cast Thrown if the property was not a T

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        template<class T> const T & ItemGet(const SwFmt &rFmt,
            sal_uInt16 eType) throw(std::bad_cast)
        {
            return item_cast<T>(rFmt.GetAttr(eType));
        }

        /** Extract a SfxPoolItem derived property from a SfxItemSet

            Writer's attributes are retrieved by passing a numeric identifier
            and receiving a SfxPoolItem reference which must then typically be
            cast back to its original type which is both tedious and verbose.

            ItemGet uses item_cast () on the retrived reference to test that the
            retrived property is of the type that the developer thinks it is.

            @param rSet
            The SfxItemSet to retrieve the property from

            @param eType
            The numeric identifier of the property to be retrieved

            @tplparam T
            A SfxPoolItem derived class of the retrieved property

            @exception std::bad_cast Thrown if the property was not a T

            @return The T requested

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        template<class T> const T & ItemGet(const SfxItemSet &rSet,
            sal_uInt16 eType) throw(std::bad_cast)
        {
            return item_cast<T>(rSet.Get(eType));
        }

        /** Extract a default SfxPoolItem derived property from a SfxItemPool

            Writer's attributes are retrieved by passing a numeric identifier
            and receiving a SfxPoolItem reference which must then typically be
            cast back to its original type which is both tedious and verbose.

            DefaultItemGet returns a reference to the default property of a
            given SfxItemPool for a given property id, e.g. default fontsize

            DefaultItemGet uses item_cast () on the retrived reference to test
            that the retrived property is of the type that the developer thinks
            it is.

            @param rPool
            The SfxItemPool whose default property we want

            @param eType
            The numeric identifier of the default property to be retrieved

            @tplparam T
            A SfxPoolItem derived class of the retrieved property

            @exception std::bad_cast Thrown if the property was not a T

            @return The T requested

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        template<class T> const T & DefaultItemGet(const SfxItemPool &rPool,
            sal_uInt16 eType) throw(std::bad_cast)
        {
            return item_cast<T>(rPool.GetDefaultItem(eType));
        }

        /** Extract a default SfxPoolItem derived property from a SwDoc

            Writer's attributes are retrieved by passing a numeric identifier
            and receiving a SfxPoolItem reference which must then typically be
            cast back to its original type which is both tedious and verbose.

            DefaultItemGet returns a reference to the default property of a
            given SwDoc (Writer Document) for a given property id, e.g default
            fontsize

            DefaultItemGet uses item_cast () on the retrived reference to test
            that the retrived property is of the type that the developer thinks
            it is.

            @param rPool
            The SfxItemPool whose default property we want

            @param eType
            The numeric identifier of the default property to be retrieved

            @tplparam T
            A SfxPoolItem derived class of the retrieved property

            @exception std::bad_cast Thrown if the property was not a T

            @return The T requested

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        template<class T> const T & DefaultItemGet(const SwDoc &rDoc,
            sal_uInt16 eType) throw(std::bad_cast)
        {
            return DefaultItemGet<T>(rDoc.GetAttrPool(), eType);
        }

        /** Return a pointer to a SfxPoolItem derived class if it exists in an
            SfxItemSet

            Writer's attributes are retrieved by passing a numeric identifier
            and receiving a SfxPoolItem reference which must then typically be
            cast back to its original type which is both tedious and verbose.

            HasItem returns a pointer to the requested SfxPoolItem for a given
            property id if it exists in the SfxItemSet or its chain of parents,
            e.g. fontsize

            HasItem uses item_cast () on the retrived pointer to test that the
            retrived property is of the type that the developer thinks it is.

            @param rSet
            The SfxItemSet whose property we want

            @param eType
            The numeric identifier of the default property to be retrieved

            @tplparam T
            A SfxPoolItem derived class of the retrieved property

            @return The T requested or 0 if no T found with id eType

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        template<class T> const T* HasItem(const SfxItemSet &rSet,
            sal_uInt16 eType)
        {
            return item_cast<T>(rSet.GetItem(eType));
        }

        /** Get the Paragraph Styles of a SwDoc

            Writer's styles are in one of those dreaded macro based pre-STL
            containers. Give me an STL container of the paragraph styles
            instead.

            @param rDoc
            The SwDoc document to get the styles from

            @return A ParaStyles containing the SwDoc's Paragraph Styles

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        ParaStyles GetParaStyles(const SwDoc &rDoc);

        /** Sort sequence of Paragraph Styles by outline numbering level

            Sort ParaStyles in ascending order of outline level, e.g.  given
            Normal/Heading2/Heading1 at their default outline levels of body
            level/level 2/level 1 sorts them to Heading1/Heading2/Normal

            @param rStyles
            The ParaStyles to sort

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        void SortByOutline(ParaStyles &rStyles);
    }

    namespace hack
    {
        /** Make setting a drawing object's layer in a Writer document easy


            Word has the simple concept of a drawing object either in the
            foreground and in the background. We have an additional complexity
            that form components live in a seperate layer, which seems
            unnecessarily complicated. So in the winword filter we set the
            object's layer through this class with either SendObjectToHell for
            the bottom layer and SendObjectToHeaven for the top and we don't
            worry about the odd form layer design wrinkle.

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        class SetLayer
        {
        private:
            sal_uInt8 mnHeavenLayer, mnHellLayer, mnFormLayer;
            enum Layer {eHeaven, eHell};
            void SetObjectLayer(SdrObject &rObject, Layer eLayer) const;
            void Swap(SetLayer &rOther) throw();
        public:

            /** Make Object live in the bottom drawing layer

                @param rObject
                The object to be set to the bottom layer
            */
            void SendObjectToHell(SdrObject &rObject) const;

            /** Make Object lives in the top top layer

                @param rObject
                The object to be set to the bottom layer
            */
            void SendObjectToHeaven(SdrObject &rObject) const;

            /** Normal constructor

                @param rDoc
                The Writer document whose drawing layers we will be inserting
                objects into
            */
            SetLayer(const SwDoc &rDoc);

            SetLayer(const SetLayer &rOther) throw();
            SetLayer& operator=(const SetLayer &rOther) throw();
        };

        /** Make inserting an OLE object into a Writer document easy

            The rest of Office uses SdrOle2Obj for their OLE objects, Writer
            doesn't, which makes things a bit difficult as this is the type of
            object that the escher import code shared by the MSOffice filters
            produces when it imports an OLE object.

            This utility class takes ownership of the OLE object away from a
            SdrOle2Obj and can massage it into the condition best suited to
            insertion into Writer.

            If the object was not transferred into Writer then it is deleted
            during destruction.

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        class DrawingOLEAdaptor
        {
        private:
            String msOrigPersistName;
            SvInPlaceObjectRef mxIPRef;
            SvPersist &mrPers;
        public:
            /** Take ownership of a SdrOle2Objs OLE object

                @param rObj
                The SdrOle2Obj whose OLE object we want to take control of

                @param rPers
                The SvPersist of a SwDoc (SwDoc::GetPersist()) into which we
                may want to move the object, or remove it from if unwanted.
            */
            DrawingOLEAdaptor(SdrOle2Obj &rObj, SvPersist &rPers);

            /// Destructor will destroy the owned OLE object if not transferred
            ~DrawingOLEAdaptor();

            /** Transfer ownership of the OLE object to a document's SvPersist

                TransferToDoc moves the object into the persist under the name
                passed in. This name is then suitable to be used as an argument
                to SwDoc::InsertOLE.

                The object is no longer owned by the adaptor after this call,
                subsequent calls are an error and return false.

                @param rName
                The name to store the object under in the document.

                @return On success true is returned, otherwise false. On
                success rName is then suitable for user with SwDoc::InsertOLE
            */
            bool TransferToDoc(const String &rName);
        private:
            /// No assigning allowed
            DrawingOLEAdaptor& operator=(const DrawingOLEAdaptor&);
            /// No copying allowed
            DrawingOLEAdaptor(const DrawingOLEAdaptor &rDoc);
        };
    }
}

#endif
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
