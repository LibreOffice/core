/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef CSI_XML_XMLITEM_HXX
#define CSI_XML_XMLITEM_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/tpl/swelist.hxx>
#include <cosv/tpl/dyn.hxx>
    // PARAMETERS

namespace csv
{
    class bostream;
}


namespace csi
{
namespace xml
{


/*  Basics:
    Item, Attribute, Element, TextContext
*/

class Item
{
  public:
    virtual             ~Item() {}
    void                WriteOut(
                            csv::bostream &     io_aFile ) const;
  private:
    virtual void        do_WriteOut(
                            csv::bostream &     io_aFile ) const = 0;
};

typedef csv::SweList_dyn< Item >            ItemList;

class Attribute
{
  public:
    virtual             ~Attribute() {}

    void                WriteOut(
                            csv::bostream &     io_aFile ) const;

    const ::csv::String& Name() const;
    const ::csv::String& Value() const;

  private:
    virtual const ::csv::String &
                        inq_Name() const = 0;
    virtual const ::csv::String &
                        inq_Value() const = 0;
};

typedef csv::SweList_dyn< Attribute >   AttrList;


class Element : public Item
{
  public:
    Element &           operator<<(             /// For multiple content items.
                            DYN Item *          let_dpItem );
    Element &           operator<<(             /// For multiple content items.
                            const ::csv::String& let_drText );
    Element &           operator<<(             /// For multiple content items.
                            const char *        let_dpText );
    Element &           operator<<(
                            DYN Attribute *     let_dpAttr );

    Element &           operator>>(             /// For multiple content items. @return the child Element.
                            DYN Element &       let_drElement );


    Item *              SetContent(             /// For only one content item.
                            DYN Item *          let_dpItem );       /// Replaces previous content. May be 0, then all content is deleted.
  private:
    // Interface Item:
    virtual void        do_WriteOut(
                            csv::bostream &     io_aFile ) const;
    // Local
    virtual void        op_streamout(
                            DYN Item *          let_dpItem ) = 0;
    virtual void        op_streamout(
                            DYN Attribute *     let_dpAttr ) = 0;

    virtual void        do_SetContent(
                            DYN Item *          let_dpItem ) = 0;
    // Helpers
    virtual const ::csv::String &
                        inq_TagName() const = 0;
    virtual const Item *
                        inq_Content() const = 0;
    virtual const AttrList *
                        inq_Attrs() const = 0;

    virtual bool        FinishEmptyTag_XmlStyle() const;    /// Defaulted to: true

    virtual bool        LineBreakAfterBeginTag() const;     /// Defaulted to: false
    virtual bool        LineBreakAfterEndTag() const;       /// Defaulted to: true, if LineBreakAfterBeginTag()
};

class TextContent : public Item
{
};


/*  Implementation simplifiers:
    EmptyElement, PureElement, SglTag
*/

class EmptyElement : public Element
{
  private:
    // Interface Element:
    virtual void        op_streamout(          /// does nothing
                            DYN Item *          let_dpItem );
    virtual void        op_streamout(
                            DYN Attribute *     let_dpAttr );
    virtual void        do_SetContent(          /// does nothing
                            DYN Item *          let_dpItem );
    virtual const Item *
                        inq_Content() const;    /// @return 0
    virtual const AttrList *
                        inq_Attrs() const;

    // Local
    virtual AttrList &  inq_RefAttrs() = 0;
};

class PureElement : public Element
{
  private:
    // Interface Element:
    virtual void        op_streamout(
                            DYN Item *          let_dpItem );
    virtual void        op_streamout(          /// does nothing
                            DYN Attribute *     let_dpAttr );
    virtual void        do_SetContent(
                            DYN Item *          let_dpItem );
    virtual const Item *
                        inq_Content() const;
    virtual const AttrList *
                        inq_Attrs() const;      /// @return 0
    // Local
    virtual Dyn< Item > &
                        inq_RefContent() = 0;
};

class SglTag : public Element
{
  private:
    // Interface Element:
    virtual void        op_streamout(          /// does nothing
                            DYN Item *          let_dpItem );
    virtual void        op_streamout(          /// does nothing
                            DYN Attribute *     let_dpAttr );
    virtual void        do_SetContent(          /// does nothing
                            DYN Item *          let_dpItem );
    virtual const Item *
                        inq_Content() const;    /// @return 0
    virtual const AttrList *
                        inq_Attrs() const;      /// @return 0
};



/*  Standard Element implementations, if there are not any
    specialized ones.
*/

class AnElement : public Element
{
  public:
                        AnElement(
                            const char *         i_sTagName );
                        ~AnElement();
  private:
    // Interface Element
    virtual void        op_streamout(
                            DYN Item *          let_dpItem );
    virtual void        op_streamout(
                            DYN Attribute *     let_dpAttr );
    virtual void        do_SetContent(
                            DYN Item *          let_dpItem );
    virtual const ::csv::String &
                        inq_TagName() const;
    virtual const Item *
                        inq_Content() const;
    virtual const AttrList *
                        inq_Attrs() const;
    // DATA
    ::csv::String       sTagName;
    Dyn< Item >         pContent;
    AttrList            aAttrs;
};

class APureElement : public PureElement
{
  public:
                        APureElement(
                            const char *            i_sTagName );
                        ~APureElement();
  private:
    // Interface Element:
    virtual const ::csv::String &
                        inq_TagName() const;
    // Interface PureElement:
    virtual Dyn< Item > &
                        inq_RefContent();
    // DATA
    ::csv::String       sTagName;
    Dyn< Item >         pContent;
};

/* Standard Attribute implementation
*/
class AnAttribute : public Attribute
{
  public:
                        AnAttribute(
                            const ::csv::String & i_sName,
                            const ::csv::String & i_sValue );
                        AnAttribute(
                            const char *          i_sName,
                            const char *          i_sValue );
                        ~AnAttribute();
  private:
    // Interface Attribute:
    virtual const ::csv::String &
                        inq_Name() const;
    virtual const ::csv::String &
                        inq_Value() const;
    // DATA
    ::csv::String         sName;
    ::csv::String         sValue;
};



/*  Implementations of TextContent:

    Text        ( reserved characters will be replaced and appear unchanged )
    XmlCode     ( reserved characters stay and are interpreted
                  by the XML-viewer )
*/
class Text : public TextContent
{
  public:
                        Text(
                            const ::csv::String & i_sText );
                        Text(
                            const char *        i_sText );
                        ~Text();
  private:
    virtual void        do_WriteOut(
                            csv::bostream &     io_aFile ) const;
    // DATA
    ::csv::String         sText;
};


class XmlCode : public TextContent
{
  public:
                        XmlCode(
                            const ::csv::String & i_sText );
                        XmlCode(
                            const char *          i_sText );
                        ~XmlCode();
  private:
    virtual void        do_WriteOut(
                            csv::bostream &     io_aFile ) const;
    // DATA
    ::csv::String         sText;
};



// IMPLEMENTATION

inline void
Item::WriteOut( csv::bostream & io_aFile ) const
    { do_WriteOut(io_aFile); }

inline const ::csv::String &
Attribute::Name() const
    { return inq_Name(); }
inline const ::csv::String &
Attribute::Value() const
    { return inq_Value(); }

inline Element &
Element::operator<<( DYN Item * let_dpItem )
    { op_streamout(let_dpItem); return *this; }
inline Element &
Element::operator<<( const ::csv::String & let_drText )
    { op_streamout( new Text(let_drText) ); return *this; }
inline Element &
Element::operator<<( const char * let_drText )
    { op_streamout( new Text(let_drText) ); return *this; }
inline Element &
Element::operator<<( DYN Attribute * let_dpAttr )
    { op_streamout(let_dpAttr); return *this; }
inline Element &
Element::operator>>( DYN Element & let_drElement )
    { op_streamout(&let_drElement); return let_drElement; }
inline Item *
Element::SetContent( DYN Item * let_dpItem )
    { do_SetContent(let_dpItem); return let_dpItem; }


}   // namespace xml
}   // namespace csi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
