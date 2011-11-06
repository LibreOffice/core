/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

