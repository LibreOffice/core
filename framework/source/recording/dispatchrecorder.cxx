
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

//***********************************************************************
DispatchRecorder::DispatchRecorder( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
        : ThreadHelpBase     ( &Application::GetSolarMutex() )
        , ::cppu::OWeakObject(                               )
        , m_xSMGR            ( xSMGR                         )
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
    WriteGuard aWriteLock(m_aLock);

    LOG_ASSERT2(m_aScriptBuffer.getLength()>0, "DispatchRecorder::startRecording()", "start without end called ... append new macro to old one!")
    m_aScriptBuffer.ensureCapacity(10000);

    m_aScriptBuffer.appendAscii("rem ----------------------------------------------------------------------\n");
    m_aScriptBuffer.appendAscii("rem define variables\n");
    m_aScriptBuffer.appendAscii("dim document   as object\n");
    m_aScriptBuffer.appendAscii("dim dispatcher as object\n");
    m_aScriptBuffer.appendAscii("dim parser     as object\n");
    m_aScriptBuffer.appendAscii("dim url        as new com.sun.star.util.URL\n");
    m_aScriptBuffer.appendAscii("rem ----------------------------------------------------------------------\n");
    m_aScriptBuffer.appendAscii("rem get access to the document\n");
    m_aScriptBuffer.appendAscii("document = ThisComponent.CurrentController.Frame\n");
    m_aScriptBuffer.appendAscii("parser   = createUnoService(\"com.sun.star.util.URLTransformer\")\n\n");

    m_nRecordingID = 1;
    /* } */
}

//*************************************************************************
void SAL_CALL DispatchRecorder::recordDispatch( const css::util::URL& aURL,
                                                const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    implts_recordMacro(aURL,lArguments,sal_False);
}

//*************************************************************************
void SAL_CALL  DispatchRecorder::recordDispatchAsComment( const css::util::URL& aURL,
                                                          const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    implts_recordMacro(aURL,lArguments,sal_True);
}

//*************************************************************************
void SAL_CALL DispatchRecorder::endRecording() throw( css::uno::RuntimeException )
{
    /* SAFE{ */
    WriteGuard aWriteLock(m_aLock);
    m_sScript = m_aScriptBuffer.makeStringAndClear();
    /* } */
}

//*************************************************************************
::rtl::OUString SAL_CALL DispatchRecorder::getRecordedMacro() throw( css::uno::RuntimeException )
{
    /* SAFE{ */
    ReadGuard aReadLock(m_aLock);
    return m_sScript;
    /* } */
}

//*************************************************************************
static void AppendToBuffer( css::uno::Any aValue, ::rtl::OUStringBuffer& aArgumentBuffer )
{
    // if value == bool
    if (aValue.getValueType() == getBooleanCppuType())
    {
        sal_Bool bVal;
        aValue >>= bVal;
        if (bVal)
            aArgumentBuffer.appendAscii("true");
        else
            aArgumentBuffer.appendAscii("false");
    }
    else
    // if value == sal_Int8
    if (aValue.getValueType() == getCppuType((sal_Int8*)0))
    {
        sal_Int8 nVal;
        aValue >>= nVal;
        aArgumentBuffer.append((sal_Int32)nVal);
    }
    else
    // if value == sal_Int16
    if (aValue.getValueType() == getCppuType((sal_Int16*)0))
    {
        sal_Int16 nVal;
        aValue >>= nVal;
        aArgumentBuffer.append((sal_Int32)nVal);
    }
    else
    // if value == sal_Int32
    if (aValue.getValueType() == getCppuType((sal_Int32*)0))
    {
        sal_Int32 nVal;
        aValue >>= nVal;
        aArgumentBuffer.append((sal_Int32)nVal);
    }
    else
    // if value == float
    if (aValue.getValueType() == getCppuType((float*)0))
    {
        float fVal;
        aValue >>= fVal;
        aArgumentBuffer.append(fVal);
    }
    else
    // if value == double
    if (aValue.getValueType() == getCppuType((double*)0))
    {
        double fVal;
        aValue >>= fVal;
        aArgumentBuffer.append(fVal);
    }
    else
    // if value == string
    if (aValue.getValueType() == getCppuType((::rtl::OUString*)0))
    {
        ::rtl::OUString sVal;
        aValue >>= sVal;
        aArgumentBuffer.appendAscii("\"");
        aArgumentBuffer.append     (sVal);
        aArgumentBuffer.appendAscii("\"");
    }
    else if (aValue.getValueType() == ::getCppuType((const css::uno::Sequence < css::uno::Any >*)0) )
    {
        css::uno::Sequence < css::uno::Any > aSeq;
        aValue >>= aSeq;
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
    else {
        LOG_WARNING("","Type not scriptable!")
    }
}

void SAL_CALL DispatchRecorder::implts_recordMacro( const css::util::URL& aURL,
                                                    const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                                          sal_Bool bAsComment )
{
    ::rtl::OUStringBuffer aArgumentBuffer(1000);
    ::rtl::OUStringBuffer aScriptBuffer(1000);
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
    aScriptBuffer.append     (aURL.Complete);
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
    m_aScriptBuffer.append(aScriptBuffer.makeStringAndClear());
    m_nRecordingID++;
    /* } */
}

} // namespace framework
