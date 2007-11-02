/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i2s_calculator.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:42:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ARY_IDL_I2S_CALCULATOR_HXX
#define ARY_IDL_I2S_CALCULATOR_HXX


// USED SERVICES
    // BASE CLASSES
#include <cosv/tpl/processor.hxx>
    // PARAMETERS
#include <ary/idl/i_types4idl.hxx>


namespace ary
{
namespace idl
{
    class CeAdmin;
    class Ce_Storage;
    class TypeAdmin;
    class Type_Storage;
    class Module;
    class ExplicitType;
    class Function;
    class Interface;
    class Property;
    class Typedef;
    class Service;
    class Singleton;
    class SglIfcService;
    class SglIfcSingleton;
    class Struct;
    class StructElement;
    class Exception;
    class Ce_2s;
}
}






namespace ary
{
namespace idl
{


enum E_2s_of_Service
{
    service_2s_IncludingServices,
    service_2s_InstantiatingSingletons
};

enum E_2s_of_Interface
{
    interface_2s_Derivations,
    interface_2s_ExportingServices,
    interface_2s_ExportingSingletons,
    interface_2s_SynonymTypedefs,
//    interface_2s_UsingTypedefs,
    interface_2s_AsReturns,
//    interface_2s_AsIndirectReturns,
    interface_2s_AsParameters,
//    interface_2s_AsIndirectParameters,
    interface_2s_AsDataTypes
};

enum E_2s_of_Struct
{
    struct_2s_Derivations,
    struct_2s_SynonymTypedefs,
//    struct_2s_UsingTypedefs,
    struct_2s_AsReturns,
//    struct_2s_AsIndirectReturns,
    struct_2s_AsParameters,
//    struct_2s_AsIndirectParameters,
    struct_2s_AsDataTypes
};

enum E_2s_of_Enum
{
    enum_2s_SynonymTypedefs,
//    enum_2s_UsingTypedefs,
    enum_2s_AsReturns,
//    enum_2s_AsIndirectReturns,
    enum_2s_AsParameters,
//    enum_2s_AsIndirectParameters,
    enum_2s_AsDataTypes
};

enum E_2s_of_Typedef
{
    typedef_2s_SynonymTypedefs,
//    typedef_2s_UsingTypedefs,
    typedef_2s_AsReturns,
//    typedef_2s_AsIndirectReturns,
    typedef_2s_AsParameters,
//    typedef_2s_AsIndirectParameters,
    typedef_2s_AsDataTypes
};

enum E_2s_of_Exceptions
{
    exception_2s_Derivations,
    exception_2s_RaisingFunctions
};



class SPInst_asHost :   public csv::ProcessorIfc,
                        public csv::ConstProcessor<Service>,
                        public csv::ConstProcessor<Interface>,
                        public csv::ConstProcessor<Struct>,
                        public csv::ConstProcessor<Exception>,
                        public csv::ConstProcessor<Typedef>,
                        public csv::ConstProcessor<Singleton>,
                        public csv::ConstProcessor<Function>,
                        public csv::ConstProcessor<StructElement>,
                        public csv::ConstProcessor<Property>,
                        public csv::ConstProcessor<SglIfcService>,
                        public csv::ConstProcessor<SglIfcSingleton>
{
};




/** This class scans the parsed data and produces several
    secondary data like cross references and alphabetical indices.

    In this declaration "Secondaries" or "2s" mean those secondary data.

    @see Ce_2s
*/
class SecondariesCalculator : public SPInst_asHost
{
  public:
    // LIFECYCLE
                        SecondariesCalculator(
                            CeAdmin &           i_ces,
                            TypeAdmin &         i_types );
    virtual             ~SecondariesCalculator();

    // OPERATIONS
    void                CheckAllInterfaceBases();
    void                Connect_Types2Ces();
    void                Gather_CrossReferences();
    void                Make_Links2DeveloperManual(
                            const String &      i_devman_reffilepath );

  private:
    // Interface CeHost These are the points to gather cross
    //   references:
    virtual void        do_Process(
                            const Service &     i_rData );
    virtual void        do_Process(
                            const Interface &   i_rData );
    virtual void        do_Process(
                            const Struct &      i_rData );
    virtual void        do_Process(
                            const Exception &   i_rData );
    virtual void        do_Process(
                            const Typedef &     i_rData );
    virtual void        do_Process(
                            const Singleton &   i_rData );
    virtual void        do_Process(
                            const Function &    i_rData );
    virtual void        do_Process(
                            const StructElement &
                                                i_rData );
    virtual void        do_Process(
                            const Property &    i_rData );
    virtual void        do_Process(
                            const SglIfcService &
                                                i_rData );
    virtual void        do_Process(
                            const SglIfcSingleton &
                                                i_rData );

    // Locals
    const Ce_Storage &  my_CeStorage() const;
    const Type_Storage &
                        my_TypeStorage() const;
    Ce_Storage &        my_CeStorage();
    Type_Storage &      my_TypeStorage();

    template <class DEST>
    DEST *              SearchCe4Type(
                            Type_id             i_type );
    Ce_id               lhf_Search_CeForType(
                            const ExplicitType &
                                                i_rType ) const;
    Ce_id               lhf_Search_CeFromTypeId(
                            Type_id             i_nType ) const;
    Service *           lhf_SearchService(
                            Type_id             i_nServ );
    Interface *         lhf_SearchInterface(
                            Type_id             i_nIfc );
    Struct *            lhf_SearchStruct(
                            Type_id             i_nIfc );
    Exception *         lhf_SearchException(
                            Type_id             i_nIfc );
    void                assign_CurLink(
                            char *              i_text,
                            const String &      i_link,
                            const String &      i_linkUI,
                            bool                i_isDescr,      /// @descr true: description, false: reference.
                            int                 i_lineCount  );
    void                gather_Synonyms();
    void                recursive_AssignAsSynonym(
                            Ce_id               i_synonymousTypedefsId,
                            const Typedef &     i_TypedefToCheck );
    void                recursive_AssignIncludingService(
                            Ce_id               i_includingServicesId,
                            const Service &     i_ServiceToCheckItsIncludes );
    void                assign_AsDerivedInterface(
                            const Interface &   i_rDerived );
    void                assign_AsDerivedStruct(
                            const Struct &      i_rDerived );
    void                assign_AsDerivedException(
                            const Exception &   i_rDerived );
    void                assignImplementation_toAServicesInterfaces(
                            Ce_id               i_nImpl,
                            Ce_id               i_nService,
                            E_2s_of_Interface   i_eList );
    void                recursive_AssignImplementation_toExportedInterface(
                            Ce_id               i_nService,
                            Type_id             i_nExportedInterface,
                            E_2s_of_Interface   i_eList );
    void                recursive_AssignFunction_toCeAsReturn(
                            Ce_id               i_nFunction,
                            Type_id             i_nReturnType );
    void                recursive_AssignFunction_toCeAsParameter(
                            Ce_id               i_nFunction,
                            Type_id             i_nParameterType );

    /** @param i_nDataElement
        May be the ID of an struct element as well as an exception element
        or a property.
    */
    void                recursive_AssignStructElement_toCeAsDataType(
                            Ce_id               i_nDataElement,
                            Type_id             i_nDataType );
    void                insert_into2sList(
                            CodeEntity &        o_out,
                            int                 i_listIndex,
                            Ce_id               i_nCe );
    void                insert_into2sUnique(
                            CodeEntity &        o_out,
                            int                 i_listIndex,
                            Ce_id               i_nCe );
    /// Sorts secondary production lists alphabetical.
    void                sort_All2s();

    void                Read_Links2DevManual(
                            csv::bstream &      i_file );

    // DATA
    CeAdmin *           pCes;
    TypeAdmin *         pTypes;
};




}   // namespace idl
}   // namespace ary
#endif
