/*************************************************************************
 *
 *  $RCSfile: xmltree.hxx,v $
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

#ifndef X2C_XMLTREE_HXX
#define X2C_XMLTREE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "../support/sistr.hxx"
#include "../support/list.hxx"
    // PARAMETERS


class ComponentDescription;
class ReferenceDocuElement;

class TextElement
{
  public:
    virtual void        SetText(
                            const char *        i_sText ) = 0;
    virtual const char *
                        Name() const = 0;
    virtual const char *
                        Data(
                            unsigned            i_nNr = 0 ) const = 0;
    virtual unsigned    Size() const = 0;
};

class ParentElement
{
  public:
    typedef DynamicList<TextElement>            ChildList;
    void                AddChild(
                            TextElement &       let_drElement );

    const ChildList &   Children() const        { return aChildren; }
    ChildList &         Children()              { return aChildren; }

  private:
    ChildList           aChildren;
};



class ModuleDescription : public ParentElement
{
  public:
    typedef DynamicList<ComponentDescription> CD_List;

    void                AddComponentDescription(
                            ComponentDescription &
                                            let_drCD );

    const char *        Name() const            { return Children()[0]->Data(); }
    const CD_List &     Components() const      { return aCDs; }

  private:
    CD_List             aCDs;
};


class ComponentDescription : public ParentElement
{
  public:
    typedef DynamicList<ReferenceDocuElement> Docu_List;

    void                SetStatus(
                            const char *        i_sText );

    const char *        Name() const            { return Children()[0]->Data(); }
    const Docu_List &   DocuRefs() const        { return aDocuRefs; }
    Docu_List &         DocuRefs()              { return aDocuRefs; }
    const char *        Status() const          { return sStatus; }

  private:
    Simstr              sStatus;
    Docu_List           aDocuRefs;
};


class SglTextElement : public TextElement
{
  public:
                        SglTextElement(
                            const char *        i_sName );
    virtual void        SetText(
                            const char *        i_sText );

    virtual const char *
                        Name() const;
    virtual const char *
                        Data(
                            unsigned            i_nNr = 0 ) const;
    virtual unsigned    Size() const;

  private:
    Simstr              sName;
    Simstr              sContent;
};


class MultipleTextElement : public TextElement
{
  public:
                        MultipleTextElement(
                            const char *        i_sName );
    virtual void        SetText(
                            const char *        i_sText );

    virtual const char *
                        Name() const;
    virtual const char *
                        Data(
                            unsigned            i_nNr = 0 ) const;
    virtual unsigned    Size() const;

  private:
    Simstr              sName;
    List<Simstr>        aContent;
};


class ReferenceDocuElement
{
  public:

    Simstr              sAttr_href;
    Simstr              sAttr_role;
    Simstr              sAttr_title;
};

// IMPLEMENTATION


#endif

