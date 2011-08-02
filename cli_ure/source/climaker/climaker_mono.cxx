#include <stdio.h>
#include <vector>
#include <memory>
#include <cstring>

#include "osl/process.h"
#include "osl/file.hxx"
#include "osl/thread.h"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "cppuhelper/shlib.hxx"
#include "cppuhelper/bootstrap.hxx"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/container/XSet.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/reflection/XConstantTypeDescription.hpp"
#include "com/sun/star/reflection/XConstantsTypeDescription.hpp"
#include "com/sun/star/reflection/XEnumTypeDescription.hpp"
#include "com/sun/star/reflection/XIndirectTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceTypeDescription2.hpp"
#include "com/sun/star/reflection/XInterfaceAttributeTypeDescription2.hpp"
#include "com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp"
#include "com/sun/star/reflection/XMethodParameter.hpp"
#include "com/sun/star/reflection/XParameter.hpp"
#include "com/sun/star/reflection/XServiceConstructorDescription.hpp"
#include "com/sun/star/reflection/XServiceTypeDescription2.hpp"
#include "com/sun/star/reflection/XSingletonTypeDescription2.hpp"
#include "com/sun/star/reflection/XStructTypeDescription.hpp"
#include "com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"

using namespace ::std;

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define OUSTR(x) ::rtl::OUString(x, strlen (x), osl_getThreadTextEncoding ())

enum UnoTypeClass {
    UTC_Unknown,
    UTC_Void,
    UTC_Char,
    UTC_Boolean,
    UTC_Byte,
    UTC_Short,
    UTC_UnsignedShort,
    UTC_Long,
    UTC_UnsignedLong,
    UTC_Hyper,
    UTC_UnsignedHyper,
    UTC_Float,
    UTC_Double,
    UTC_String,
    UTC_Type,
    UTC_Any,
    UTC_Enum,
    UTC_Typedef,
    UTC_Struct,
    UTC_Exception,
    UTC_Sequence,
    UTC_Interface,
    UTC_InterfaceAttribute,
    UTC_InterfaceMethod,
    UTC_Constant,
    UTC_Constants,
    UTC_Service,
    UTC_Singleton,
    UTC_Module
};

UnoTypeClass unoTypeClassToCLI (TypeClass tc)
{
    switch (tc) {
    case TypeClass_UNKNOWN:
        return UTC_Unknown;
    case TypeClass_VOID:
        return UTC_Void;
    case TypeClass_CHAR:
        return UTC_Char;
    case TypeClass_BOOLEAN:
        return UTC_Boolean;
    case TypeClass_BYTE:
        return UTC_Byte;
    case TypeClass_SHORT:
        return UTC_Short;
    case TypeClass_UNSIGNED_SHORT:
        return UTC_UnsignedShort;
    case TypeClass_LONG:
        return UTC_Long;
    case TypeClass_UNSIGNED_LONG:
        return UTC_UnsignedLong;
    case TypeClass_HYPER:
        return UTC_Hyper;
    case TypeClass_UNSIGNED_HYPER:
        return UTC_UnsignedHyper;
    case TypeClass_FLOAT:
        return UTC_Float;
    case TypeClass_DOUBLE:
        return UTC_Double;
    case TypeClass_STRING:
        return UTC_String;
    case TypeClass_TYPE:
        return UTC_Type;
    case TypeClass_ANY:
        return UTC_Any;
    case TypeClass_ENUM:
        return UTC_Enum;
    case TypeClass_TYPEDEF:
        return UTC_Typedef;
    case TypeClass_STRUCT:
        return UTC_Struct;
    case TypeClass_EXCEPTION:
        return UTC_Exception;
    case TypeClass_SEQUENCE:
        return UTC_Sequence;
    case TypeClass_INTERFACE:
        return UTC_Interface;
    case TypeClass_INTERFACE_ATTRIBUTE:
        return UTC_InterfaceAttribute;
    case TypeClass_INTERFACE_METHOD:
        return UTC_InterfaceMethod;
    case TypeClass_CONSTANT:
        return UTC_Constant;
    case TypeClass_CONSTANTS:
        return UTC_Constants;
    case TypeClass_SERVICE:
        return UTC_Service;
    case TypeClass_SINGLETON:
        return UTC_Singleton;
    case TypeClass_MODULE:
        return UTC_Module;
    default:
        break;
    }

    throw RuntimeException(
        OUSTR("unexpected type in unoTypeClassToCLI"),
        Reference< XInterface >() );
}

UnoTypeClass unoTypeClass (reflection::XTypeDescription* xType)
{
    return unoTypeClassToCLI (xType->getTypeClass ());
}

class UnoHelper
{
    Reference< XComponentContext > mxContext;
    Reference< lang::XSingleComponentFactory > mxTDprov_factory;
    Reference< container::XHierarchicalNameAccess > mxTDmgr;
    Reference< XInterface > mxTD_provider;
    Reference< reflection::XTypeDescriptionEnumeration > mxTD_enum;
    Reference< reflection::XTypeDescription > mxTD;
    sal_uInt32 mnPos;

    vector< OUString > mMandatoryRegistries;
    vector< OUString > mExtraRegistries;
    vector< OUString > mExplicitTypes;

    static OUString const & path_get_working_dir()
    {
        static OUString s_workingDir;
        if (! s_workingDir.getLength())
            osl_getProcessWorkingDir( &s_workingDir.pData );
        return s_workingDir;
    }

    static OUString path_make_absolute_file_url( OUString const & path )
    {
        OUString file_url;
        oslFileError rc = osl_getFileURLFromSystemPath(
            path.pData, &file_url.pData );
        if (osl_File_E_None == rc)
        {
            OUString abs;
            rc = osl_getAbsoluteFileURL(
                path_get_working_dir().pData, file_url.pData, &abs.pData );
            if (osl_File_E_None == rc)
            {
                return abs;
            }
            else
            {
                throw RuntimeException(
                    OUSTR("cannot make absolute: ") + file_url,
                    Reference< XInterface >() );
            }
        }
        else
        {
            throw RuntimeException(
                OUSTR("cannot get file url from system path: ") + path,
                Reference< XInterface >() );
        }
    }

    Reference< registry::XSimpleRegistry > open_registries(
        vector< OUString > const & registries )
    {
        if (registries.empty())
        {
            throw RuntimeException(OUSTR("no registries given!"),
                                   Reference< XInterface >() );
        }

        Reference< registry::XSimpleRegistry > xSimReg;
        for ( size_t nPos = registries.size(); nPos--; )
        {
            Reference< registry::XSimpleRegistry > xReg(
                mxContext->getServiceManager()->createInstanceWithContext(
                    OUSTR("com.sun.star.registry.SimpleRegistry"), mxContext ),
                UNO_QUERY_THROW );
            xReg->open( registries[ nPos ], sal_True, sal_False );
            if (! xReg->isValid())
            {
                throw RuntimeException(
                    OUSTR("invalid registry: ") + registries[ nPos ],
                    Reference< XInterface >() );
            }

            if (xSimReg.is()) // nest?
            {
                Reference< registry::XSimpleRegistry > xNested(
                    mxContext->getServiceManager()->createInstanceWithContext(
                        OUSTR("com.sun.star.registry.NestedRegistry"), mxContext ),
                    UNO_QUERY_THROW );
                Reference< lang::XInitialization > xInit(
                    xNested, UNO_QUERY_THROW );
                Sequence< Any > args( 2 );
                args[ 0 ] <<= xReg;
                args[ 1 ] <<= xSimReg;
                xInit->initialize( args );
                xSimReg = xNested;
            }
            else
            {
                xSimReg = xReg;
            }
        }

        return xSimReg;
    }

    public:

    UnoHelper ()
    {
        // bootstrap uno
        mxContext = ::cppu::bootstrap_InitialComponentContext(
            Reference< registry::XSimpleRegistry >() );
        mxTDmgr = Reference< container::XHierarchicalNameAccess > (
            mxContext->getValueByName(
                OUSTR("/singletons/com.sun.star.reflection."
                      "theTypeDescriptionManager") ),
            UNO_QUERY_THROW );

        // get rdb tdprovider factory
        mxTDprov_factory = Reference< lang::XSingleComponentFactory > (
            ::cppu::loadSharedLibComponentFactory(
                OUSTR("bootstrap.uno" SAL_DLLEXTENSION), OUString(),
                OUSTR("com.sun.star.comp.stoc.RegistryTypeDescriptionProvider"),
                Reference< lang::XMultiServiceFactory >(
                    mxContext->getServiceManager(), UNO_QUERY ),
                Reference< registry::XRegistryKey >() ), UNO_QUERY );
        if (! mxTDprov_factory.is())
        {
            throw RuntimeException(
                OUSTR("cannot get registry typedescription provider: "
                      "bootstrap.uno" SAL_DLLEXTENSION "!"),
                Reference< XInterface >() );
        }
    }

    void AddMandatoryRegistry (char *registry)
    {
        mMandatoryRegistries.push_back(
            path_make_absolute_file_url( OUSTR( registry ) ) );
    }

    void AddExtraRegistry (char *registry)
    {
        mExtraRegistries.push_back(
            path_make_absolute_file_url( OUSTR( registry ) ) );
    }

    void AddExplicitType (char *typeName)
    {
        mExplicitTypes.push_back( OUSTR( typeName ) );
    }

    void OpenRegistries ()
    {
        // create registry td provider for mandatory registry files
        Any arg( makeAny( open_registries( mMandatoryRegistries ) ) );
        mxTD_provider = Reference< XInterface >(
            mxTDprov_factory->createInstanceWithArgumentsAndContext(
                Sequence< Any >( &arg, 1 ), mxContext ) );

        // insert provider to tdmgr
        Reference< container::XSet > xSet( mxTDmgr, UNO_QUERY_THROW );
        Any provider( makeAny( mxTD_provider ) );
        xSet->insert( provider );
        OSL_ASSERT( xSet->has( provider ) );

        if (! mExtraRegistries.empty())
        {
            arg = makeAny( open_registries( mExtraRegistries ) );
            provider = makeAny(
                mxTDprov_factory->createInstanceWithArgumentsAndContext(
                    Sequence< Any >( &arg, 1 ), mxContext ) );
            xSet->insert( provider );
            OSL_ASSERT( xSet->has( provider ) );
        }

        // and emit types to it
        if (mExplicitTypes.empty())
        {
            mxTD_enum = Reference< reflection::XTypeDescriptionEnumeration > (
                Reference< reflection::XTypeDescriptionEnumerationAccess >(
                    mxTD_provider, UNO_QUERY_THROW )
                        ->createTypeDescriptionEnumeration(
                             OUString() /* all IDL modules */,
                             Sequence< TypeClass >() /* all classes of types */,
                             reflection::TypeDescriptionSearchDepth_INFINITE ) );
        } else
            mnPos = 0;
    }

    void* NextType ()
    {
        Reference< reflection::XTypeDescription > xTD;

        if (mnPos < mExplicitTypes.size()) {
             Reference< container::XHierarchicalNameAccess > xHNA(
                 mxTD_provider, UNO_QUERY_THROW );
             xTD = Reference< reflection::XTypeDescription >(
                       xHNA->getByHierarchicalName( mExplicitTypes[ mnPos ] ),
                       UNO_QUERY_THROW );
             mnPos ++;
        } else if (mxTD_enum.is() && mxTD_enum->hasMoreElements())
            xTD = Reference< reflection::XTypeDescription >(mxTD_enum->nextTypeDescription());

        if (xTD.is()) {
            xTD.get ()->acquire ();

            return xTD.get ();
        }

        return NULL;
    }

    ~UnoHelper ()
    {
    }
};

extern "C" {

    static char* oustr_to_char (OUString oustr)
    {
        OString os = OUStringToOString (oustr, osl_getThreadTextEncoding());

        size_t len = os.getLength () + 1;
        char *str = (char *) malloc (len);
        memcpy (str, os.getStr (), len);

        return str;
    }

    /* UnoHelper class glue */

    void* SAL_CALL
    cmm_uno_helper_new ()
    {
        return new UnoHelper ();
    }

    void SAL_CALL
    cmm_uno_helper_add_mandatory_registry (void *ptr, char *registry)
    {
        UnoHelper *helper = (UnoHelper *) ptr;

        helper->AddMandatoryRegistry (registry);
    }

    void SAL_CALL
    cmm_uno_helper_add_extra_registry (void *ptr, char *registry)
    {
        UnoHelper *helper = (UnoHelper *) ptr;

        helper->AddExtraRegistry (registry);
    }

    void SAL_CALL
    cmm_uno_helper_add_explicit_type (void *ptr, char *typeName)
    {
        UnoHelper *helper = (UnoHelper *) ptr;

        helper->AddExplicitType (typeName);
    }

    void SAL_CALL
    cmm_uno_helper_open_registries (void *ptr)
    {
        UnoHelper *helper = (UnoHelper *) ptr;

        helper->OpenRegistries ();
    }

    void* SAL_CALL
    cmm_uno_helper_next_type (void *ptr)
    {
        UnoHelper *helper = (UnoHelper *) ptr;

        return helper->NextType ();
    }

    void SAL_CALL
    cmm_uno_helper_delete (void *ptr)
    {
        UnoHelper *helper = (UnoHelper *) ptr;

        delete helper;
    }

    /* XTypeDescription glue */

    char * SAL_CALL
    cmm_x_type_description_get_name (void *ptr)
    {
        reflection::XTypeDescription* xTD = (reflection::XTypeDescription *) ptr;

        return oustr_to_char (xTD->getName ());
    }

    int SAL_CALL
    cmm_x_type_description_get_type_class (void *ptr)
    {
        reflection::XTypeDescription* xTD = (reflection::XTypeDescription *) ptr;

        return unoTypeClass (xTD);
    }

    void SAL_CALL
    cmm_x_type_description_release (void *ptr)
    {
        reflection::XTypeDescription* xTD = (reflection::XTypeDescription *) ptr;

        xTD->release ();
    }

    /* XEnumTypeDescription glue */

     char * SAL_CALL
     cmm_x_enum_type_description_get_name (void *ptr, int idx)
     {
         reflection::XEnumTypeDescription* xTD = (reflection::XEnumTypeDescription *) ptr;

         Sequence< OUString > seq_enum_names (xTD->getEnumNames ());

         return oustr_to_char (seq_enum_names [idx]);
     }

     int SAL_CALL
     cmm_x_enum_type_description_get_value (void *ptr, int idx)
     {
         reflection::XEnumTypeDescription* xTD = (reflection::XEnumTypeDescription *) ptr;

         Sequence< sal_Int32 > seq_enum_values (xTD->getEnumValues ());

         return seq_enum_values [idx];
     }

    int SAL_CALL
    cmm_x_enum_type_description_get_length (void *ptr)
    {
        reflection::XEnumTypeDescription* xTD = (reflection::XEnumTypeDescription *) ptr;

        Sequence< sal_Int32 > seq_enum_values (xTD->getEnumValues ());

        return seq_enum_values.getLength ();
    }

    /* XInterfaceTypeDescription glue */

    int SAL_CALL
    cmm_x_interface_type_description_get_members (void *ptr, const void **handle)
    {
        reflection::XInterfaceTypeDescription* xTD = (reflection::XInterfaceTypeDescription *) ptr;

        Sequence<
            Reference< reflection::XInterfaceMemberTypeDescription > > seq_members (xTD->getMembers ());
        Reference< reflection::XInterfaceMemberTypeDescription > const * types = seq_members.getConstArray();

        int length = seq_members.getLength ();
        reflection::XInterfaceMemberTypeDescription **array = (reflection::XInterfaceMemberTypeDescription**) malloc (length*sizeof (reflection::XInterfaceMemberTypeDescription *));

        *handle = array;

        for (int i = 0; i < length; i ++) {
            array [i] = types [i].get ();
            array [i]->acquire ();
        }

        return length;
    }

    void * SAL_CALL
    cmm_x_interface_type_description_get_member (void *ptr, int idx)
    {
        reflection::XInterfaceMemberTypeDescription** members = (reflection::XInterfaceMemberTypeDescription**) ptr;

        members [idx]->acquire ();

        return members [idx];
    }

    void SAL_CALL
    cmm_x_interface_type_description_members_release (void *ptr)
    {
//         uno_Sequence* seq = (uno_Sequence*) ptr;

//         seq->release ();
    }

    /* XInterfaceTypeDescription2 glue */

    void * SAL_CALL
    cmm_resolveInterface (Reference< reflection::XTypeDescription > type)
    {
        Reference<reflection::XInterfaceTypeDescription2>
            xIfaceTd (type, UNO_QUERY);

        if (xIfaceTd.is()) {
            reflection::XTypeDescription* xtd = type.get ();

            xtd->acquire ();

            return xtd;
        } else {
            Reference<reflection::XIndirectTypeDescription> xIndTd(
                                                                   type, UNO_QUERY);
            if (xIndTd.is() == sal_False)
                throw uno::Exception(
                                     OUSTR("resolveInterfaceTypedef was called with an invalid argument"), 0);
            return cmm_resolveInterface (xIndTd->getReferencedType());
        }
    }


    void* SAL_CALL
    cmm_x_interface_type_description_get_base_type (void *ptr, int idx)
    {
        reflection::XInterfaceTypeDescription2* xTD = (reflection::XInterfaceTypeDescription2 *) ptr;

        Sequence< Reference< reflection::XTypeDescription > > seq_base_types (xTD->getBaseTypes ());

        Reference< reflection::XTypeDescription > type (seq_base_types [idx]);

        return cmm_resolveInterface (type);
    }

    int SAL_CALL
    cmm_x_interface_type_description_get_length (void *ptr)
    {
        reflection::XInterfaceTypeDescription2* xTD = (reflection::XInterfaceTypeDescription2 *) ptr;

        Sequence< Reference< reflection::XTypeDescription > > seq_base_types (xTD->getBaseTypes ());

        return seq_base_types.getLength ();
    }

    /* XInterfaceAttributeTypeDescription glue */

    void* SAL_CALL
    cmm_x_interface_attribute_type_description_get_type (void *ptr)
    {
        reflection::XInterfaceAttributeTypeDescription2* xTD = (reflection::XInterfaceAttributeTypeDescription2 *) ptr;
        Reference< reflection::XTypeDescription > xrt = xTD->getType ();

        xrt.get ()->acquire ();

        return xrt.get ();
    }

    bool SAL_CALL
    cmm_x_interface_attribute_type_description_is_bound (void *ptr)
    {
        reflection::XInterfaceAttributeTypeDescription2* xTD = (reflection::XInterfaceAttributeTypeDescription2 *) ptr;

        return xTD->isBound ();
    }

    bool SAL_CALL
    cmm_x_interface_attribute_type_description_is_read_only (void *ptr)
    {
        reflection::XInterfaceAttributeTypeDescription2* xTD = (reflection::XInterfaceAttributeTypeDescription2 *) ptr;

        return xTD->isReadOnly ();
    }

    static int
    get_exception_types (const Sequence<Reference< reflection::XCompoundTypeDescription > >& seq_exceptionsCTD, void **handle)
    {
        int length = seq_exceptionsCTD.getLength ();

        if (length <= 0)
            return 0;

        Reference< reflection::XCompoundTypeDescription > const * types = seq_exceptionsCTD.getConstArray();
        size_t size = length*sizeof (reflection::XCompoundTypeDescription *);
        reflection::XCompoundTypeDescription **array = (reflection::XCompoundTypeDescription**) malloc (size);

        *handle = array;

        for (int i = 0; i < length; i ++) {
            array [i] = types [i].get ();
            array [i]->acquire ();
        }

        return length;
    }

    int SAL_CALL
    cmm_x_interface_attribute_type_description_get_get_exception_types (void *ptr, void **handle)
    {
        reflection::XInterfaceAttributeTypeDescription2* xTD = (reflection::XInterfaceAttributeTypeDescription2 *) ptr;

        return get_exception_types (xTD->getGetExceptions (), handle);
    }

    int SAL_CALL
    cmm_x_interface_attribute_type_description_get_set_exception_types (void *ptr, void **handle)
    {
        reflection::XInterfaceAttributeTypeDescription2* xTD = (reflection::XInterfaceAttributeTypeDescription2 *) ptr;

        return get_exception_types (xTD->getSetExceptions (), handle);
    }

    void* SAL_CALL
    cmm_x_interface_attribute_type_description_get_exception_type (void *ptr, int index)
    {
        reflection::XCompoundTypeDescription **array = (reflection::XCompoundTypeDescription**) ptr;;

        return array [index];
    }

    static void
    free_exception_types (void *ptr, int count)
    {
        reflection::XCompoundTypeDescription **array = (reflection::XCompoundTypeDescription**) ptr;;
        int i;

        for (i = 0; i < count; i ++)
            array [i]->release ();

        free (array);
    }

    void SAL_CALL
    cmm_x_interface_attribute_type_description_free_exception_types (void *ptr, int count)
    {
        free_exception_types (ptr, count);
    }

    /* XInterfaceMemberTypeDescription glue */

    char* SAL_CALL
    cmm_x_interface_member_type_description_get_member_name (void *ptr)
    {
        reflection::XInterfaceMemberTypeDescription* xTD = (reflection::XInterfaceMemberTypeDescription *) ptr;

        return oustr_to_char (xTD->getMemberName ());;
    }

    /* XInterfaceMethodTypeDescription glue */

    int SAL_CALL
    cmm_x_interface_method_type_description_get_parameters (void *ptr, const void **parametersHandle, const void** parametersArrayHandle)
    {
        reflection::XInterfaceMethodTypeDescription* xTD = (reflection::XInterfaceMethodTypeDescription *) ptr;

        Sequence<
            Reference< reflection::XMethodParameter > > seq_parameters (xTD->getParameters ());
        Reference< reflection::XMethodParameter > const * parameters = seq_parameters.getConstArray();

        *parametersHandle = seq_parameters.get ();

        int length = seq_parameters.getLength();
        size_t size = length*sizeof (Reference< reflection::XMethodParameter > const);
        Reference< reflection::XMethodParameter > *copy = (Reference< reflection::XMethodParameter > *) malloc (size);
        memcpy (copy, parameters, size);

        *parametersArrayHandle = copy;

        for (int i=0; i<length; i++) {
            copy[i].get()->acquire ();
        }

        return length;
    }

    void * SAL_CALL
    cmm_x_interface_method_type_description_get_parameter (void *ptr, int idx)
    {
        Reference< reflection::XMethodParameter > const * parameters = (Reference< reflection::XMethodParameter > const *) ptr;

        reflection::XMethodParameter* parameterHandle = parameters [idx].get ();

        parameterHandle->acquire ();

        return parameterHandle;
    }

    int SAL_CALL
    cmm_x_interface_method_type_description_get_exception_types (void *ptr, const void **exceptionTypesHandle, const void** exceptionTypesArrayHandle)
    {
        reflection::XInterfaceMethodTypeDescription* xMethod = (reflection::XInterfaceMethodTypeDescription *) ptr;

        const Sequence<Reference<reflection::XTypeDescription> > seqTD = xMethod->getExceptions();
        int length = seqTD.getLength();
        Sequence<Reference<reflection::XCompoundTypeDescription> > seqCTD( length );
        Reference< reflection::XCompoundTypeDescription > * arCTD = seqCTD.getArray();

        if (length > 0) {
            size_t size = length*sizeof (Reference< reflection::XCompoundTypeDescription > const);
            Reference< reflection::XCompoundTypeDescription > *copy = (Reference< reflection::XCompoundTypeDescription > *) malloc (size);

            for (int i=0; i<length; i++) {
                arCTD[i] = Reference<reflection::XCompoundTypeDescription>(seqTD[i], UNO_QUERY_THROW);
            }

            memcpy (copy, arCTD, size);

            *exceptionTypesArrayHandle = copy;

            for (int i=0; i<length; i++) {
                copy[i].get()->acquire ();
            }
        } else {
            *exceptionTypesArrayHandle = NULL;
            *exceptionTypesHandle = NULL;
        }

        return length;
    }

    void * SAL_CALL
    cmm_x_interface_method_type_description_get_exception_type (void *ptr, int idx)
    {
        Reference< reflection::XCompoundTypeDescription > const * exceptionTypes = (Reference< reflection::XCompoundTypeDescription > const *) ptr;

        reflection::XCompoundTypeDescription* exceptionTypeHandle = exceptionTypes [idx].get ();

        exceptionTypeHandle->acquire ();

        return exceptionTypeHandle;
    }

    void* SAL_CALL
    cmm_x_interface_method_type_description_get_return_type (void *ptr)
    {
        reflection::XInterfaceMethodTypeDescription* xTD = (reflection::XInterfaceMethodTypeDescription *) ptr;
        Reference< reflection::XTypeDescription > xrt = xTD->getReturnType ();

        xrt.get ()->acquire ();

        return xrt.get ();
    }

    bool SAL_CALL
    cmm_x_interface_method_type_description_returns_struct (void *ptr)
    {
        reflection::XInterfaceMethodTypeDescription* xTD = (reflection::XInterfaceMethodTypeDescription *) ptr;
        Reference< reflection::XStructTypeDescription > xrt (xTD->getReturnType (), UNO_QUERY);

        return xrt.is ();
    }

    bool SAL_CALL
    cmm_x_interface_method_type_description_is_oneway (void *ptr)
    {
        reflection::XInterfaceMethodTypeDescription* xTD = (reflection::XInterfaceMethodTypeDescription *) ptr;

        return xTD->isOneway ();
    }

    /* XMethodParameter glue */

    bool SAL_CALL
    cmm_x_method_parameter_is_out (void *ptr)
    {
        reflection::XMethodParameter* p = (reflection::XMethodParameter *) ptr;

        return p->isOut ();
    }

    bool SAL_CALL
    cmm_x_method_parameter_is_in (void *ptr)
    {
        reflection::XMethodParameter* p = (reflection::XMethodParameter *) ptr;

        return p->isIn ();
    }

    int SAL_CALL
    cmm_x_method_parameter_position (void *ptr)
    {
        reflection::XMethodParameter* p = (reflection::XMethodParameter *) ptr;

        return p->getPosition ();
    }

    char* SAL_CALL
    cmm_x_method_parameter_name (void *ptr)
    {
        reflection::XMethodParameter* p = (reflection::XMethodParameter *) ptr;

        return oustr_to_char (p->getName ());
    }

    void* SAL_CALL
    cmm_x_method_parameter_type (void *ptr)
    {
        reflection::XMethodParameter* p = (reflection::XMethodParameter *) ptr;
        Reference< reflection::XTypeDescription > xtd = p->getType ();

        xtd.get ()->acquire ();

        return xtd.get ();
    }

    /* XCompoundTypeDescription glue */

    void* SAL_CALL
    cmm_x_compound_type_description_get_base_type (void *ptr)
    {
        reflection::XCompoundTypeDescription* xTD = (reflection::XCompoundTypeDescription *) ptr;
        Reference< reflection::XTypeDescription > xrt = xTD->getBaseType ();

        if (!xrt.is ())
            return NULL;

        xrt.get ()->acquire ();

        return xrt.get ();
    }

    int SAL_CALL
    cmm_x_compound_type_description_get_member_names (void *ptr, const void **handle)
    {
        reflection::XCompoundTypeDescription* xTD = (reflection::XCompoundTypeDescription *) ptr;

        Sequence< OUString > seq_member_names = xTD->getMemberNames ();
        int length = seq_member_names.getLength ();

        if (length <= 0)
            return 0;

        OUString const * parameters = seq_member_names.getConstArray();
        char **array = (char**) malloc (length*sizeof (char *));

        *handle = array;

        for (int i = 0; i < length; i ++) {
            array [i] = oustr_to_char (parameters [i]);
        }

        return length;
    }

    void * SAL_CALL
    cmm_x_compound_type_description_get_member_name (void *ptr, int idx)
    {
        char** member_names = (char **) ptr;

        return strdup (member_names [idx]);
    }

    int SAL_CALL
    cmm_x_compound_type_description_get_member_types (void *ptr, const void **handle)
    {
        reflection::XCompoundTypeDescription* xTD = (reflection::XCompoundTypeDescription *) ptr;

        Sequence< Reference< reflection::XTypeDescription > > seq_member_types = xTD->getMemberTypes ();
        int length = seq_member_types.getLength ();

        if (length <= 0)
            return 0;

        Reference< reflection::XTypeDescription > const * types = seq_member_types.getConstArray();
        reflection::XTypeDescription **array = (reflection::XTypeDescription**) malloc (length*sizeof (reflection::XTypeDescription*));

        *handle = array;

        for (int i = 0; i < length; i ++) {
            array [i] = types [i].get ();
            array [i]->acquire ();
        }

        return length;
    }

    void * SAL_CALL
    cmm_x_compound_type_description_get_member_type (void *ptr, int idx)
    {
        reflection::XTypeDescription** member_types = (reflection::XTypeDescription **) ptr;
        member_types [idx]->acquire ();

        return member_types [idx];
    }

    /* XStructTypeDescription glue */

    int SAL_CALL
    cmm_x_struct_type_description_get_type_parameters (void *ptr, const void **handle)
    {
        reflection::XStructTypeDescription* xTD = (reflection::XStructTypeDescription *) ptr;

        Sequence< OUString > seq_type_parameters = xTD->getTypeParameters ();
        int length = seq_type_parameters.getLength ();

        if (length <= 0)
            return 0;

        OUString const * parameters = seq_type_parameters.getConstArray();
        char **array = (char**) malloc (length*sizeof (char *));

        *handle = array;

        for (int i = 0; i < length; i ++) {
            array [i] = oustr_to_char (parameters [i]);
        }

        return length;
    }

    void * SAL_CALL
    cmm_x_struct_type_description_get_type_parameter (void *ptr, int idx)
    {
        char** type_parameters = (char **) ptr;

        return strdup (type_parameters [idx]);
    }

    int SAL_CALL
    cmm_x_struct_type_description_get_type_arguments (void *ptr, const void **typeArgumentsHandle, const void** typeArgumentsArrayHandle)
    {
        reflection::XStructTypeDescription* xTD = (reflection::XStructTypeDescription *) ptr;

        Sequence<
            Reference< reflection::XTypeDescription > > seq_type_arguments (xTD->getTypeArguments ());
        Reference< reflection::XTypeDescription > const * type_arguments = seq_type_arguments.getConstArray();

        *typeArgumentsHandle = seq_type_arguments.get ();

        int length = seq_type_arguments.getLength();
        size_t size = length*sizeof (Reference< reflection::XTypeDescription > const);
        Reference< reflection::XTypeDescription > *copy = (Reference< reflection::XTypeDescription > *) malloc (size);
        memcpy (copy, type_arguments, size);

        *typeArgumentsArrayHandle = copy;

        for (int i=0; i<length; i++) {
            copy[i].get()->acquire ();
        }

        return length;
    }

    void * SAL_CALL
    cmm_x_struct_type_description_get_type_argument (void *ptr, int idx)
    {
        Reference< reflection::XTypeDescription > const * type_arguments = (Reference< reflection::XTypeDescription > const *) ptr;

        reflection::XTypeDescription* typeArgumentHandle = type_arguments [idx].get ();

        typeArgumentHandle->acquire ();

        return typeArgumentHandle;
    }

    /* XIndirectTypeDescription glue */

    void* SAL_CALL
    cmm_x_indirect_type_description_get_referenced_type (void *ptr)
    {
        reflection::XIndirectTypeDescription* xTD = (reflection::XIndirectTypeDescription *) ptr;
        Reference< reflection::XTypeDescription > xrt = xTD->getReferencedType ();

        xrt.get ()->acquire ();

        return xrt.get ();
    }

    /* XConstantTypeDescription glue */

    UnoTypeClass SAL_CALL
    cmm_any_to_cli_constant (void *ptr,
                             sal_Unicode *tChar, sal_Bool *tBool,
                             sal_uInt8 *tByte,
                             sal_Int16 *tInt16, sal_uInt16 *tUInt16,
                             sal_Int32 *tInt32, sal_uInt32 *tUInt32,
                             sal_Int64 *tInt64, sal_uInt64 *tUInt64,
                             float *tFloat, double *tDouble)
    {
        reflection::XConstantTypeDescription* xtd = (reflection::XConstantTypeDescription*) ptr;
        Any const & value (xtd->getConstantValue ());

        switch (value.getValueTypeClass ()) {
        case TypeClass_CHAR:
            *tChar = *reinterpret_cast< sal_Unicode const * > (value.getValue ());
            break;
        case TypeClass_BOOLEAN:
            *tBool = *reinterpret_cast< sal_Bool const * > (value.getValue ());
        case TypeClass_BYTE:
            *tByte = *reinterpret_cast< sal_uInt8 const * > (value.getValue ());
            break;
        case TypeClass_SHORT:
            *tInt16 = *reinterpret_cast< sal_Int16 const * > (value.getValue ());
            break;
        case TypeClass_UNSIGNED_SHORT:
            *tUInt16 = *reinterpret_cast< sal_uInt16 const * > (value.getValue ());
            break;
        case TypeClass_LONG:
            *tInt32 = *reinterpret_cast< sal_Int32 const * > (value.getValue ());
            break;
        case TypeClass_UNSIGNED_LONG:
            *tUInt32 = *reinterpret_cast< sal_uInt32 const * > (value.getValue ());
            break;
        case TypeClass_HYPER:
            *tInt64 = *reinterpret_cast< sal_Int64 const * > (value.getValue ());
            break;
        case TypeClass_UNSIGNED_HYPER:
            *tUInt64 = *reinterpret_cast< sal_uInt64 const * > (value.getValue ());
            break;
        case TypeClass_FLOAT:
            *tFloat = *reinterpret_cast< float const * > (value.getValue ());
            break;
        case TypeClass_DOUBLE:
            *tDouble = *reinterpret_cast< double const * > (value.getValue ());
            break;
        default:
            throw RuntimeException(OUSTR("unexpected constant type ") +
                                   value.getValueType().getTypeName(),
                                   Reference< XInterface >() );
        }

        return unoTypeClassToCLI (value.getValueTypeClass ());
    }

    /* XConstantsTypeDescription glue */

    int SAL_CALL
    cmm_x_constants_type_description_get_constants (void *ptr, const void **handle)
    {
        reflection::XConstantsTypeDescription* xTD = (reflection::XConstantsTypeDescription *) ptr;

        Sequence< Reference< reflection::XConstantTypeDescription > > seq_member_types = xTD->getConstants ();
        int length = seq_member_types.getLength ();

        if (length <= 0)
            return 0;

        Reference< reflection::XConstantTypeDescription > const * types = seq_member_types.getConstArray();
        reflection::XConstantTypeDescription **array = (reflection::XConstantTypeDescription**) malloc (length*sizeof (reflection::XConstantTypeDescription*));

        *handle = array;

        for (int i = 0; i < length; i ++) {
            array [i] = types [i].get ();
            array [i]->acquire ();
        }

        return length;
    }

    void * SAL_CALL
    cmm_x_constants_type_description_get_constant (void *ptr, int idx)
    {
        reflection::XConstantTypeDescription** array = (reflection::XConstantTypeDescription **) ptr;
        array [idx]->acquire ();

        return array [idx];
    }

    /* XServiceTypeDescription glue */

    bool SAL_CALL
    cmm_x_service_type_description_is_single_interface_based (void *ptr)
    {
        reflection::XServiceTypeDescription2* xTD = (reflection::XServiceTypeDescription2 *) ptr;

        return xTD->isSingleInterfaceBased ();
    }

    void* SAL_CALL
    cmm_x_service_type_description_get_interface (void *ptr)
    {
        reflection::XServiceTypeDescription2* xTD = (reflection::XServiceTypeDescription2 *) ptr;
        reflection::XTypeDescription* xTDiface = xTD->getInterface ().get ();

        xTDiface->acquire ();

        return xTDiface;
    }

    int SAL_CALL
    cmm_x_service_type_description_get_constructors (void *ptr, const void **handle)
    {
        reflection::XServiceTypeDescription2* xTD = (reflection::XServiceTypeDescription2 *) ptr;

        Sequence< Reference< reflection::XServiceConstructorDescription > > seq_member_types = xTD->getConstructors ();
        int length = seq_member_types.getLength ();

        if (length <= 0)
            return 0;

        Reference< reflection::XServiceConstructorDescription > const * types = seq_member_types.getConstArray();
        reflection::XServiceConstructorDescription **array = (reflection::XServiceConstructorDescription**) malloc (length*sizeof (reflection::XServiceConstructorDescription*));

        *handle = array;

        for (int i = 0; i < length; i ++) {
            array [i] = types [i].get ();
            array [i]->acquire ();
        }

        return length;
    }

    void * SAL_CALL
    cmm_x_service_type_description_get_constructor (void *ptr, int idx)
    {
        reflection::XServiceConstructorDescription** array = (reflection::XServiceConstructorDescription **) ptr;
        array [idx]->acquire ();

        return array [idx];
    }

    /* XServiceConstructorDescription glue */

    int SAL_CALL
    cmm_x_service_constructor_description_get_parameters (void *ptr, const void **handle)
    {
        reflection::XServiceConstructorDescription* xTD = (reflection::XServiceConstructorDescription *) ptr;

        Sequence< Reference< reflection::XParameter > > seq_member_types = xTD->getParameters ();
        int length = seq_member_types.getLength ();

        if (length <= 0)
            return 0;

        Reference< reflection::XParameter > const * types = seq_member_types.getConstArray();
        reflection::XParameter **array = (reflection::XParameter**) malloc (length*sizeof (reflection::XParameter*));

        *handle = array;

        for (int i = 0; i < length; i ++) {
            array [i] = types [i].get ();
            array [i]->acquire ();
        }

        return length;
    }

    void * SAL_CALL
    cmm_x_service_constructor_description_get_parameter (void *ptr, int idx)
    {
        reflection::XParameter** array = (reflection::XParameter **) ptr;
        array [idx]->acquire ();

        return array [idx];
    }

    char * SAL_CALL
    cmm_x_service_constructor_description_get_name (void *ptr)
    {
        reflection::XServiceConstructorDescription* xTD = (reflection::XServiceConstructorDescription *) ptr;

        return oustr_to_char (xTD->getName ());
    }

    bool SAL_CALL
    cmm_x_service_constructor_description_is_default_constructor (void *ptr)
    {
        reflection::XServiceConstructorDescription* xTD = (reflection::XServiceConstructorDescription *) ptr;

        return xTD->isDefaultConstructor ();
    }

    int SAL_CALL
    cmm_x_service_constructor_description_get_exception_types (void *ptr, void **handle)
    {
        reflection::XServiceConstructorDescription* xTD = (reflection::XServiceConstructorDescription *) ptr;

        return get_exception_types (xTD->getExceptions (), handle);
    }

    void* SAL_CALL
    cmm_x_service_constructor_description_get_exception_type (void *ptr, int index)
    {
        reflection::XCompoundTypeDescription **array = (reflection::XCompoundTypeDescription**) ptr;;

        return array [index];
    }

    void SAL_CALL
    cmm_x_service_constructor_description_free_exception_types (void *ptr, int count)
    {
        free_exception_types (ptr, count);
    }

    /* XParameter glue */

    bool SAL_CALL
    cmm_x_parameter_is_rest (void *ptr)
    {
        reflection::XParameter* xTD = (reflection::XParameter *) ptr;

        return xTD->isRestParameter ();
    }

    /* XServiceTypeDescription glue */

    bool SAL_CALL
    cmm_x_singleton_type_description_is_interface_based (void *ptr)
    {
        reflection::XSingletonTypeDescription2* xTD = (reflection::XSingletonTypeDescription2 *) ptr;

        return xTD->isInterfaceBased ();
    }

    void* SAL_CALL
    cmm_x_singleton_type_description_get_interface (void *ptr)
    {
        reflection::XSingletonTypeDescription2* xTD = (reflection::XSingletonTypeDescription2 *) ptr;
        Reference< reflection::XTypeDescription > xrt = xTD->getInterface ();

        xrt.get ()->acquire ();

        return xrt.get ();
    }
}
