/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
