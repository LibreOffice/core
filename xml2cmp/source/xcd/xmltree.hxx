/*************************************************************************
 *
 *  $RCSfile: xmltree.hxx,v $
 *
 *  $Revision: 1.4 $
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
#include "xmlelem.hxx"
    // COMPONENTS
#include "../support/sistr.hxx"
#include "../support/list.hxx"
    // PARAMETERS


class CompDescrList;

class ModuleDescription : public SequenceElement
{
  public:
                        ModuleDescription();

    const Simstr &      ModuleName() const;
    void                Get_SupportedServices(    /// @return also the children of component-description.
                            List< const MultipleTextElement * > &
                                                o_rServices ) const;
    void                Get_Types(
                            List< const MultipleTextElement * > &
                                                o_rTypes ) const;
    void                Get_ServiceDependencies(
                            List< const MultipleTextElement * > &
                                                o_rServices ) const;
  private:
    SglTextElement *    pModuleName;
    CompDescrList *     pCdList;
    MultipleTextElement *
                        pTypes;
    MultipleTextElement *
                        pServiceDependencies;
};


class ComponentDescription : public SequenceElement
{
  public:
                        ComponentDescription();

    const Simstr &      ComponentName() const   { return pComponentName->Data(); }
    const MultipleTextElement &
                        SupportedServices() const
                                                { return *pSupportedServices; }
    const MultipleTextElement &
                        Types() const           { return *pTypes; }
    const MultipleTextElement &
                        ServiceDependencies() const
                                                { return *pServiceDependencies; }
  private:
    SglTextElement *    pComponentName;
    MultipleTextElement *
                        pSupportedServices;
    MultipleTextElement *
                        pTypes;
    MultipleTextElement *
                        pServiceDependencies;
};

class CompDescrList : public ListElement
{
  public:
                        CompDescrList();
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const;
    virtual XmlElement *
                        Create_and_Add_NewElement();

    void                Get_SupportedServices(
                            List< const MultipleTextElement * > &
                                                o_rResult ) const;
    void                Get_Types(
                            List< const MultipleTextElement * > &
                                                o_rResult ) const;
    void                Get_ServiceDependencies(
                            List< const MultipleTextElement * > &
                                                o_rResult ) const;
  private:
    List< ComponentDescription * >
                        aCDs;
};

class MdName : public SglTextElement
{
  public:
                        MdName();
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const;
};

class CdName : public SglTextElement
{
  public:
                        CdName();
    virtual void        Write2Html(
                            HtmlCreator &       io_rHC ) const;
};

class SupportedService : public MultipleTextElement
{
  public:
                        SupportedService();

    virtual void        Insert2Index(
                            Index &             o_rIndex ) const;
};


// IMPLEMENTATION


#endif

