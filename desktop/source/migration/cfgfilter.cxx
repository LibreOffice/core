#include "cfgfilter.hxx"

#include <com/sun/star/beans/NamedValue.hpp>
#include <unotools/textsearch.hxx>
#include <tools/lang.hxx>

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::configuration::backend;

namespace desktop {

CConfigFilter::CConfigFilter(const strings_v* include,  const strings_v* exclude)
    : m_pvInclude(include)
    , m_pvExclude(exclude)
{
}

void SAL_CALL CConfigFilter::initialize(const Sequence< Any >& seqArgs)
        throw (Exception)
{
    NamedValue nv;
    for (sal_Int32 i=0; i < seqArgs.getLength(); i++)
    {
        if (seqArgs[i] >>= nv)
        {
            if (nv.Name.equalsAscii("Source"))
                nv.Value >>= m_xSourceLayer;
            if (nv.Name.equalsAscii("ComponentName"))
                nv.Value >>= m_aCurrentComponent;
        }
    }
    if (m_aCurrentComponent.getLength() == 0)
        m_aCurrentComponent = OUString::createFromAscii("unknown.component");
    if (!m_xSourceLayer.is())
        throw Exception();
}


void CConfigFilter::pushElement(rtl::OUString aName, sal_Bool bUse)
{
    OUString aPath;
    if (!m_elementStack.empty())
    {
        aPath = m_elementStack.top().path; // or use base path
        aPath += OUString::createFromAscii("/");
    }
    aPath += aName;

    // create element
    element elem;
    elem.name = aName;
    elem.path = aPath;
    elem.use = bUse;
    m_elementStack.push(elem);
}

sal_Bool CConfigFilter::checkCurrentElement()
{
    return m_elementStack.top().use;
}

static sal_Bool _checkRegexp(const OUString& aString, const OUString aPattern)
{
    using namespace utl;
    SearchParam param(aPattern, SearchParam::SRCH_REGEXP);
    TextSearch ts(param, LANGUAGE_DONTKNOW);
    xub_StrLen start = 0;
    xub_StrLen end = 0;
    end = (xub_StrLen)(aString.getLength());
    return ts.SearchFrwrd(aString, &start, &end);
}

sal_Bool CConfigFilter::checkElement(rtl::OUString aName)
{

    sal_Bool bResult = sal_False;

    // get full pathname for element
    OUString aFullPath;
    if (!m_elementStack.empty())
        aFullPath = m_elementStack.top().path + OUString::createFromAscii("/");

    aFullPath += aName;
#if 1
    // check whether any include patterns patch this path
    strings_v::const_iterator i_in = m_pvInclude->begin();
    while (i_in != m_pvInclude->end())
    {
        if (i_in->indexOf(aFullPath.copy(0, i_in->getLength()>aFullPath.getLength()
            ? aFullPath.getLength() : i_in->getLength())) == 0) // pattern is beginning of path
        {
            bResult = sal_True;
            break; // one match is enough
        }
        i_in++;
    }
    // if match is found, check for exclusion
    if (bResult)
    {
        strings_v::const_iterator i_ex = m_pvExclude->begin();
        while (i_ex != m_pvExclude->end())
        {
            if (i_in->indexOf(aFullPath.copy(0, i_ex->getLength()>aFullPath.getLength()
                ? aFullPath.getLength() : i_ex->getLength())) == 0) // pattern is beginning of path
            {
                bResult = sal_False;
                break; // one is enough...
            }
            i_ex++;
        }
    }

#else
    // check whether any include patterns patch this path
    strings_v::const_iterator i_in = m_pvInclude->begin();
    while (i_in != m_pvInclude->end())
    {
        if (_checkRegexp(aFullPath, *i_in))
        {
            bResult = sal_True;
            break; // one match is enough
        }
        i_in++;
    }

    // if match is found, check for exclusion
    if (bResult)
    {
        strings_v::const_iterator i_ex = m_pvExclude->begin();
        while (i_ex != m_pvExclude->end())
        {
            if (_checkRegexp(aFullPath, *i_ex))
            {
                bResult = sal_False;
                break; // one is enough...
            }
            i_ex++;
        }
    }
#endif
    return bResult;
}

void CConfigFilter::popElement()
{
    m_elementStack.pop();
}


void SAL_CALL CConfigFilter::readData(
        const Reference< configuration::backend::XLayerHandler >& layerHandler)
    throw (
        com::sun::star::lang::NullPointerException, lang::WrappedTargetException,
        com::sun::star::configuration::backend::MalformedDataException)
{
    // when readData is called, the submitted handler will be stored
    // in m_xLayerHandler. we will then submit ourself as a handler to
    // the SourceLayer in m_xSourceLayer.
    // when the source calls our handler functions we will use the patterns that
    // where given in the ctor to decide whther they should be relaied to the caller

    if (m_xSourceLayer.is() && layerHandler.is())
    {
        m_xLayerHandler = layerHandler;
        m_xSourceLayer->readData(Reference<XLayerHandler>(static_cast< XLayerHandler* >(this)));
    } else
    {
        throw NullPointerException();
    }
}

// XLayerHandler
void SAL_CALL CConfigFilter::startLayer()
    throw(::com::sun::star::lang::WrappedTargetException)
{
    m_xLayerHandler->startLayer();
}

void SAL_CALL CConfigFilter::endLayer()
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    m_xLayerHandler->endLayer();
}

void SAL_CALL CConfigFilter::overrideNode(
        const OUString& aName,
        sal_Int16 aAttributes,
        sal_Bool bClear)
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkElement(aName))
    {
        m_xLayerHandler->overrideNode(aName, aAttributes, bClear);
        pushElement(aName);
    }
    else
        pushElement(aName, sal_False);
}

void SAL_CALL CConfigFilter::addOrReplaceNode(
        const OUString& aName,
        sal_Int16 aAttributes)
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkElement(aName))
    {
        m_xLayerHandler->addOrReplaceNode(aName, aAttributes);
        pushElement(aName);
    }
    else
        pushElement(aName, sal_False);
}

void SAL_CALL  CConfigFilter::addOrReplaceNodeFromTemplate(
        const OUString& aName,
        const com::sun::star::configuration::backend::TemplateIdentifier& aTemplate,
        sal_Int16 aAttributes )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkElement(aName))
    {
        m_xLayerHandler->addOrReplaceNodeFromTemplate(aName, aTemplate, aAttributes);
        pushElement(aName);
    }
    else
        pushElement(aName, sal_False);
}

void SAL_CALL  CConfigFilter::endNode()
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkCurrentElement())
    {
        m_xLayerHandler->endNode();
    }
    popElement();
}

void SAL_CALL  CConfigFilter::dropNode(
        const OUString& aName )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    // does not get pushed
    if (checkElement(aName))
    {
        m_xLayerHandler->dropNode(aName);
    }
}

void SAL_CALL  CConfigFilter::overrideProperty(
        const OUString& aName,
        sal_Int16 aAttributes,
        const Type& aType,
        sal_Bool bClear )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkElement(aName)){
        m_xLayerHandler->overrideProperty(aName, aAttributes, aType, bClear);
        pushElement(aName);
    }
    else
        pushElement(aName, sal_False);
}

void SAL_CALL  CConfigFilter::setPropertyValue(
        const Any& aValue )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkCurrentElement())
        m_xLayerHandler->setPropertyValue(aValue);

    // setting value ends the property
    // popElement();

}

void SAL_CALL CConfigFilter::setPropertyValueForLocale(
        const Any& aValue,
        const OUString& aLocale )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkCurrentElement())
        m_xLayerHandler->setPropertyValueForLocale(aValue, aLocale);

    // setting value ends the property
    // popElement();

}

void SAL_CALL  CConfigFilter::endProperty()
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkCurrentElement())
    {
        m_xLayerHandler->endProperty();
    }
    popElement();

}

void SAL_CALL  CConfigFilter::addProperty(
        const rtl::OUString& aName,
        sal_Int16 aAttributes,
        const Type& aType )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkElement(aName))
    {
        m_xLayerHandler->addProperty(aName, aAttributes, aType);
        pushElement(aName);
    }
    else
        pushElement(aName, sal_False);
}

void SAL_CALL  CConfigFilter::addPropertyWithValue(
        const rtl::OUString& aName,
        sal_Int16 aAttributes,
        const Any& aValue )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    // add property with value doesn't push the property
    if (checkElement(aName))
        m_xLayerHandler->addPropertyWithValue(aName, aAttributes, aValue);

}

} // namespace desktop
