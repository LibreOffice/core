
#ifndef __FRAMEWORK_RECORDING_DISPATCHRECORDER_HXX_
#include <recording/dispatchrecorder.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace ::com::sun::star::uno;

namespace framework{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_3(
    DispatchRecorder,
    OWeakObject,
    DIRECT_INTERFACE(css::lang::XTypeProvider),
    DIRECT_INTERFACE(css::lang::XServiceInfo),
    DIRECT_INTERFACE(css::frame::XDispatchRecorder))

DEFINE_XTYPEPROVIDER_3(
    DispatchRecorder,
    css::lang::XTypeProvider,
    css::lang::XServiceInfo,
    css::frame::XDispatchRecorder)

DEFINE_XSERVICEINFO_MULTISERVICE(
    DispatchRecorder,
    ::cppu::OWeakObject,
    SERVICENAME_DISPATCHRECORDER,
    IMPLEMENTATIONNAME_DISPATCHRECORDER)

DEFINE_INIT_SERVICE(
    DispatchRecorder,
    {
    }
)

#include <typelib/typedescription.h>

//--------------------------------------------------------------------------------------------------
void flatten_struct_members(
    ::std::vector< Any > * vec, void const * data,
    typelib_CompoundTypeDescription * pTD )
    SAL_THROW( () )
{
    if (pTD->pBaseTypeDescription)
    {
        flatten_struct_members( vec, data, pTD->pBaseTypeDescription );
    }
    for ( sal_Int32 nPos = 0; nPos < pTD->nMembers; ++nPos )
    {
        vec->push_back(
            Any( (char const *)data + pTD->pMemberOffsets[ nPos ], pTD->ppTypeRefs[ nPos ] ) );
    }
}
//==================================================================================================
Sequence< Any > make_seq_out_of_struct(
    Any const & val )
    SAL_THROW( (RuntimeException) )
{
    Type const & type = val.getValueType();
    TypeClass eTypeClass = type.getTypeClass();
    if (TypeClass_STRUCT != eTypeClass && TypeClass_EXCEPTION != eTypeClass)
    {
        throw RuntimeException(
            type.getTypeName() +
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("is no struct or exception!") ),
            Reference< XInterface >() );
    }
    typelib_TypeDescription * pTD = 0;
    TYPELIB_DANGER_GET( &pTD, type.getTypeLibType() );
    OSL_ASSERT( pTD );
    if (! pTD)
    {
        throw RuntimeException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("cannot get type descr of type ") ) +
            type.getTypeName(),
            Reference< XInterface >() );
    }

    ::std::vector< Any > vec;
    vec.reserve( ((typelib_CompoundTypeDescription *)pTD)->nMembers ); // good guess
    flatten_struct_members( &vec, val.getValue(), (typelib_CompoundTypeDescription *)pTD );
    TYPELIB_DANGER_RELEASE( pTD );
    return Sequence< Any >( &vec[ 0 ], vec.size() );
}


struct DispatchStatement
{
    ::rtl::OUString aCommand;
    css::uno::Sequence < css::beans::PropertyValue > aArgs;
    sal_Bool bIsComment;

    DispatchStatement( const ::rtl::OUString& rCmd, const css::uno::Sequence< css::beans::PropertyValue >& rArgs, sal_Bool bComment )
        : aCommand( rCmd )
        , bIsComment( bComment )
        {
            aArgs = rArgs;
        }
};

//***********************************************************************
DispatchRecorder::DispatchRecorder( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
        : ThreadHelpBase     ( &Application::GetSolarMutex() )
        , ::cppu::OWeakObject(                               )
        , m_xSMGR            ( xSMGR                         )
        , m_xConverter( m_xSMGR->createInstance(::rtl::OUString::createFromAscii("com.sun.star.script.Converter")), css::uno::UNO_QUERY )
{
}

//************************************************************************
DispatchRecorder::~DispatchRecorder()
{
}

//*************************************************************************
// generate header
void SAL_CALL DispatchRecorder::startRecording( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException )
{
    /* SAFE{ */
    /* } */
}

//*************************************************************************
void SAL_CALL DispatchRecorder::recordDispatch( const css::util::URL& aURL,
                                                const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    //implts_recordMacro(aURL,lArguments,sal_False);
    sal_Int32 nSize = m_aStatements.size();
    if ( nSize && aURL.Complete.compareToAscii(".uno:InsertText") == COMPARE_EQUAL )
    {
        DispatchStatementList::reverse_iterator pLast = m_aStatements.rbegin();
        if ( pLast->aCommand.compareToAscii(".uno:InsertText") == COMPARE_EQUAL )
        {
            ::rtl::OUString aStr;
            ::rtl::OUString aNew;
            pLast->aArgs[0].Value >>= aStr;
            lArguments[0].Value >>= aNew;
            aStr += aNew;
            pLast->aArgs[0].Value <<= aStr;
            return;
        }
    }

    DispatchStatement aStatement( aURL.Complete, lArguments, sal_False );
    m_aStatements.push_back( aStatement );
}

//*************************************************************************
void SAL_CALL  DispatchRecorder::recordDispatchAsComment( const css::util::URL& aURL,
                                                          const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    //implts_recordMacro(aURL,lArguments,sal_True);
    DispatchStatement aStatement( aURL.Complete, lArguments, sal_True );
    m_aStatements.push_back( aStatement );
}

//*************************************************************************
void SAL_CALL DispatchRecorder::endRecording() throw( css::uno::RuntimeException )
{
    /* SAFE{ */
    WriteGuard aWriteLock(m_aLock);
    m_aStatements.clear();
    /* } */
}

//*************************************************************************
::rtl::OUString SAL_CALL DispatchRecorder::getRecordedMacro() throw( css::uno::RuntimeException )
{
    /* SAFE{ */
    WriteGuard aWriteLock(m_aLock);
    ::rtl::OUStringBuffer aScriptBuffer;
    aScriptBuffer.ensureCapacity(10000);
    m_nRecordingID = 1;

    aScriptBuffer.appendAscii("rem ----------------------------------------------------------------------\n");
    aScriptBuffer.appendAscii("rem define variables\n");
    aScriptBuffer.appendAscii("dim document   as object\n");
    aScriptBuffer.appendAscii("dim dispatcher as object\n");
    aScriptBuffer.appendAscii("dim parser     as object\n");
    aScriptBuffer.appendAscii("dim url        as new com.sun.star.util.URL\n");
    aScriptBuffer.appendAscii("rem ----------------------------------------------------------------------\n");
    aScriptBuffer.appendAscii("rem get access to the document\n");
    aScriptBuffer.appendAscii("document = ThisComponent.CurrentController.Frame\n");
    aScriptBuffer.appendAscii("parser   = createUnoService(\"com.sun.star.util.URLTransformer\")\n\n");

    std::vector< DispatchStatement>::iterator p;
    for ( p = m_aStatements.begin(); p != m_aStatements.end(); p++ )
        implts_recordMacro( p->aCommand, p->aArgs, p->bIsComment, aScriptBuffer );
    ::rtl::OUString sScript = aScriptBuffer.makeStringAndClear();
    return sScript;
    /* } */
}

//*************************************************************************
void SAL_CALL DispatchRecorder::AppendToBuffer( css::uno::Any aValue, ::rtl::OUStringBuffer& aArgumentBuffer )
{
    // if value == bool
    if (aValue.getValueTypeClass() == css::uno::TypeClass_STRUCT )
    {
        // structs are recorded as arrays, convert to "Sequence of any"
        Sequence< Any > aSeq = make_seq_out_of_struct( aValue );
        aArgumentBuffer.appendAscii("Array(");
        for ( sal_Int32 nAny=0; nAny<aSeq.getLength(); nAny++ )
        {
            AppendToBuffer( aSeq[nAny], aArgumentBuffer );
            if ( nAny+1 < aSeq.getLength() )
                // not last argument
                aArgumentBuffer.appendAscii(",");
        }

        aArgumentBuffer.appendAscii(")");
    }
    else if (aValue.getValueTypeClass() == css::uno::TypeClass_SEQUENCE )
    {
        // convert to "Sequence of any"
        css::uno::Sequence < css::uno::Any > aSeq;
        css::uno::Any aNew;
        try { aNew = m_xConverter->convertTo( aValue, ::getCppuType((const css::uno::Sequence < css::uno::Any >*)0) ); }
        catch (css::uno::Exception&) {}

        aNew >>= aSeq;
        aArgumentBuffer.appendAscii("Array(");
        for ( sal_Int32 nAny=0; nAny<aSeq.getLength(); nAny++ )
        {
            AppendToBuffer( aSeq[nAny], aArgumentBuffer );
            if ( nAny+1 < aSeq.getLength() )
                // not last argument
                aArgumentBuffer.appendAscii(",");
        }

        aArgumentBuffer.appendAscii(")");
    }
    else if (aValue.getValueTypeClass() == css::uno::TypeClass_STRING )
    {
        // strings need \"
        ::rtl::OUString sVal;
        aValue >>= sVal;

        // encode non printable characters or '"' by using the CHR$ function
        const sal_Unicode* pChars = sVal.getStr();
        sal_Bool bInString = sal_False;
        for ( sal_Int32 nChar=0; nChar<sVal.getLength(); nChar ++ )
        {
            if ( pChars[nChar] < 32 || pChars[nChar] == '"' )
            {
                // problematic character detected
                if ( bInString )
                {
                    // close current string
                    aArgumentBuffer.appendAscii("\"");
                    bInString = sal_False;
                }

                if ( nChar>0 )
                    // if this is not the first character, parts of the string have already been added
                    aArgumentBuffer.appendAscii("+");

                // add the character constant
                aArgumentBuffer.appendAscii("CHR$(");
                aArgumentBuffer.append( (sal_Int32) pChars[nChar] );
                aArgumentBuffer.appendAscii(")");
            }
            else
            {
                if ( !bInString )
                {
                    if ( nChar>0 )
                        // if this is not the first character, parts of the string have already been added
                        aArgumentBuffer.appendAscii("+");

                    // start a new string
                    aArgumentBuffer.appendAscii("\"");
                    bInString = sal_True;
                }

                aArgumentBuffer.append( pChars[nChar] );
            }
        }

        // close string
        if ( bInString )
            aArgumentBuffer.appendAscii("\"");
    }
    else if (aValue.getValueType() == getCppuCharType())
    {
        // character variables are recorded as strings, back conversion must be handled in client code
        sal_Unicode nVal = *((sal_Unicode*)aValue.getValue());
        aArgumentBuffer.appendAscii("\"");
        if ( (sal_Unicode(nVal) == '\"') )
            // encode \" to \"\"
            aArgumentBuffer.append((sal_Unicode)nVal);
        aArgumentBuffer.append((sal_Unicode)nVal);
        aArgumentBuffer.appendAscii("\"");
    }
    else
    {
        css::uno::Any aNew;
        try
        {
            aNew = m_xConverter->convertToSimpleType( aValue, css::uno::TypeClass_STRING );
        }
        catch (css::script::CannotConvertException&) { LOG_WARNING("","Type not scriptable!") }
        catch (css::uno::Exception&) {}
        ::rtl::OUString sVal;
        aNew >>= sVal;

        if (aValue.getValueTypeClass() == css::uno::TypeClass_ENUM )
        {
            ::rtl::OUString aName = aValue.getValueType().getTypeName();
            aArgumentBuffer.append( aName );
            aArgumentBuffer.appendAscii(".");
        }

        aArgumentBuffer.append(sVal);
    }
}

void SAL_CALL DispatchRecorder::implts_recordMacro( const ::rtl::OUString& aURL,
                                                    const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                                          sal_Bool bAsComment, ::rtl::OUStringBuffer& aScriptBuffer )
{
    ::rtl::OUStringBuffer aArgumentBuffer(1000);
    ::rtl::OUString       sArrayName;

    aScriptBuffer.appendAscii("rem ----------------------------------------------------------------------\n");
    aScriptBuffer.appendAscii("rem dispatch\n");

    sal_Int32 nLength = lArguments.getLength();
    sal_Int32 nValidArgs = 0;
    for( sal_Int32 i=0; i<nLength; ++i )
    {
        if(lArguments[i].Value.hasValue())
        {
            // this value is used to name the arrays of aArgumentBuffer
            if(sArrayName.getLength()<1)
            {
                sArrayName = ::rtl::OUString::createFromAscii("args");
                sArrayName += ::rtl::OUString::valueOf((sal_Int32)m_nRecordingID);
            }

            ++nValidArgs;

            // add arg().Name
            if(bAsComment)
                aArgumentBuffer.appendAscii("rem ");
            aArgumentBuffer.append     (sArrayName);
            aArgumentBuffer.appendAscii("(");
            aArgumentBuffer.append     (i);
            aArgumentBuffer.appendAscii(").Name = \"");
            aArgumentBuffer.append     (lArguments[i].Name);
            aArgumentBuffer.appendAscii("\"\n");

            // add arg().Value
            if(bAsComment)
                aArgumentBuffer.appendAscii("rem ");
            aArgumentBuffer.append     (sArrayName);
            aArgumentBuffer.appendAscii("(");
            aArgumentBuffer.append     (i);
            aArgumentBuffer.appendAscii(").Value = ");
            AppendToBuffer( lArguments[i].Value, aArgumentBuffer);
            aArgumentBuffer.appendAscii("\n");
        }
    }

    // if aArgumentBuffer exist - pack it into the aScriptBuffer
    if(nValidArgs>0)
    {
        if(bAsComment)
            aArgumentBuffer.appendAscii("rem ");
        aScriptBuffer.appendAscii("dim ");
        aScriptBuffer.append     (sArrayName);
        aScriptBuffer.appendAscii("(");
        aScriptBuffer.append     ((sal_Int32)(nValidArgs-1)); // 0 based!
        aScriptBuffer.appendAscii(") as new com.sun.star.beans.PropertyValue\n");
        aScriptBuffer.append     (aArgumentBuffer.makeStringAndClear());
        aScriptBuffer.appendAscii("\n");
    }

    // add code for parsing urls
    if(bAsComment)
        aArgumentBuffer.appendAscii("rem ");
    aScriptBuffer.appendAscii("url.Complete = \"");
    aScriptBuffer.append     (aURL);
    aScriptBuffer.appendAscii("\"\n");
    if(bAsComment)
        aArgumentBuffer.appendAscii("rem ");
    aScriptBuffer.appendAscii("parser.parseStrict(url)\n");

    // add code for dispatches
    if(bAsComment)
        aArgumentBuffer.appendAscii("rem ");
    aScriptBuffer.appendAscii("disp = document.queryDispatch(url,\"\",0)\n");
    if(bAsComment)
        aArgumentBuffer.appendAscii("rem ");
    if(nValidArgs<1)
        aScriptBuffer.appendAscii("disp.dispatch(url,noargs())\n");
    else
    {
        aScriptBuffer.appendAscii("disp.dispatch(url,");
        aScriptBuffer.append( sArrayName.getStr() );
        aScriptBuffer.appendAscii("())\n");
    }

    aScriptBuffer.appendAscii("\n");

    /* SAFE { */
    m_nRecordingID++;
    /* } */
}

} // namespace framework
