/*************************************************************************
 *
 *  $RCSfile: xmlelem.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $Date:  $
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
class Index;

class XmlElement
{
  public:
    virtual             ~XmlElement() {}
    virtual void        Parse(
                            X2CParser &         io_rParser ) = 0;
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const = 0;
    virtual void        Insert2Index(
                            Index &             o_rIndex ) const;           // Default: Does nothing, but can be overwritten.

//  virtual void        Put2Dependy() = 0;

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

