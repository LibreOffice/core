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

#ifndef X2C_XMLELEM_HXX
#define X2C_XMLELEM_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "../support/sistr.hxx"
#include "../support/list.hxx"
#include "../support/syshelp.hxx"
    // PARAMETERS


class X2CParser;
class HtmlCreator;

class XmlElement
{
  public:
    virtual             ~XmlElement() {}
    virtual void        Parse(
                            X2CParser &         io_rParser ) = 0;
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const = 0;

    const Simstr &      Name() const            { return sName; }

  protected:
                        XmlElement(
                            const char *        i_sName );
  private:
    Simstr              sName;
};


class MultipleElement : public XmlElement
{
  public:
                        ~MultipleElement();

    virtual XmlElement *
                        FindChild(
                            const Simstr &      i_sChildName );


  protected:
    typedef DynamicList<XmlElement>         ChildList;

                        MultipleElement(
                            const char *        i_sName );

    void                AddChild(
                            XmlElement &        let_drElement );

    const ChildList &   Children() const        { return aChildren; }
    ChildList &         Children()              { return aChildren; }

  private:
    ChildList           aChildren;
};

class SequenceElement : public MultipleElement
{
  public:
    virtual void        Parse(
                            X2CParser &         io_rParser );
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const;

  protected:
                        SequenceElement(
                            const char *        i_sName,
                            unsigned            i_nIndexNameElement = 0 );
  private:
    unsigned            nIndexNameElement;
};

class FreeChoiceElement : public MultipleElement
{
  public:
                        FreeChoiceElement();
    using               MultipleElement::AddChild;

    virtual void        Parse(
                            X2CParser &         io_rParser );
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const;
};

class ListElement : public MultipleElement
{
  public:
    typedef XmlElement * (*F_CREATE)(const Simstr &);

                        ListElement(
                            const char *        i_sElementsName,
                            F_CREATE            i_fCreateNewElement );

    virtual void        Parse(
                            X2CParser &         io_rParser );
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const;
    virtual XmlElement *
                        Create_and_Add_NewElement();
  private:
    F_CREATE            fCreateNewElement;
};

class TextElement : public XmlElement
{
  public:
    E_LinkType          LinkType() const        { return eLinkType; }
    bool                IsReversedName() const  { return bReverseName; }
  protected:
                        TextElement(
                            const char *        i_sName,
                            E_LinkType          i_eLinkType,
                            bool                i_bReverseName );
  private:
    E_LinkType          eLinkType;
    bool                bReverseName;
};

class SglTextElement : public TextElement
{
  public:
                        SglTextElement(
                            const char *        i_sName,
                            E_LinkType          i_eLinkType,
                            bool                i_bReverseName );

    virtual void        Parse(
                            X2CParser &         io_rParser );
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const;
    virtual const Simstr &
                        Data() const            { return sContent; }
  private:
    Simstr              sContent;
};

class MultipleTextElement : public TextElement
{
  public:
                        MultipleTextElement(
                            const char *        i_sName,
                            E_LinkType          i_eLinkType,
                            bool                i_bReverseName );

    virtual void        Parse(
                            X2CParser &         io_rParser );
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const;
    virtual const Simstr &
                        Data(
                            unsigned            i_nNr ) const;
    virtual unsigned    Size() const            { return aContent.size(); }

  private:
    List<Simstr>        aContent;
};

class EmptyElement : public XmlElement
{
  protected:
                        EmptyElement(
                            const char *        i_sName );
};

class SglAttrElement : public EmptyElement
{
  public:
                        SglAttrElement(
                            const char *        i_sName,
                            const char *        i_sAttrName );

    virtual void        Parse(
                            X2CParser &         io_rParser );
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const;
  private:
    Simstr              sAttrName;
    Simstr              sAttrValue;
};


class MultipleAttrElement : public EmptyElement
{
  public:
                        MultipleAttrElement(
                            const char *        i_sName,
                            const char **       i_sAttrNames,
                            unsigned            i_nSize );

    virtual void        Parse(
                            X2CParser &         io_rParser );
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const;
  private:
    List<Simstr>        aAttrNames;
    List<Simstr>        aAttrValues;
};

// IMPLEMENTATION


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
