#ifndef GCONFLAYER_HXX_
#define GCONFLAYER_HXX_

#include "gconfbackend.hxx"
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/util/XTimeStamped.hpp>
#include <cppuhelper/implbase2.hxx>

#define GCONF_PROXY_MODE_KEY "/system/proxy/mode"
#define GCONF_AUTO_SAVE_KEY  "/apps/openoffice/auto_save"

enum ConfigurationSetting
{
    SETTING_PROXY_MODE,
    SETTING_PROXY_HTTP_HOST,
    SETTING_PROXY_HTTP_PORT,
    SETTING_PROXY_HTTPS_HOST,
    SETTING_PROXY_HTTPS_PORT,
    SETTING_PROXY_FTP_HOST,
    SETTING_PROXY_FTP_PORT,
    SETTING_NO_PROXY_FOR,
    SETTING_ENABLE_ACCESSIBILITY,
    SETTING_MAILER_PROGRAM,
    SETTING_WORK_DIRECTORY,
    SETTING_SOURCEVIEWFONT_NAME,
    SETTING_SOURCEVIEWFONT_HEIGHT,
    SETTING_USER_GIVENNAME,
    SETTING_USER_SURNAME,

#ifdef ENABLE_LOCKDOWN

    SETTING_DISABLE_PRINTING,
    SETTING_USE_SYSTEM_FILE_DIALOG,
    SETTING_DISABLE_UI_CUSTOMIZATION,
    SETTING_PRINTING_MODIFIES_DOCUMENT,
    SETTING_SHOW_ICONS_IN_MENUS,
    SETTING_SHOW_INACTIVE_MENUITEMS,
    SETTING_SHOW_FONT_PREVIEW,
    SETTING_SHOW_FONT_HISTORY,
    SETTING_ENABLE_OPENGL,
    SETTING_OPTIMIZE_OPENGL,
    SETTING_SAVE_DOCUMENT_WINDOWS,
    SETTING_SAVE_DOCUMENT_VIEW_INFO,
    SETTING_USE_SYSTEM_FONT,
    SETTING_USE_FONT_ANTI_ALIASING,
    SETTING_FONT_ANTI_ALIASING_MIN_PIXEL,
    SETTING_WARN_CREATE_PDF,
    SETTING_WARN_PRINT_DOC,
    SETTING_WARN_SAVEORSEND_DOC,
    SETTING_WARN_SIGN_DOC,
    SETTING_REMOVE_PERSONAL_INFO,
    SETTING_RECOMMEND_PASSWORD,
    SETTING_UNDO_STEPS,
    SETTING_SYMBOL_SET,
    SETTING_MACRO_SECURITY_LEVEL,
    SETTING_CREATE_BACKUP,
    SETTING_WARN_ALIEN_FORMAT,
    SETTING_AUTO_SAVE,
    SETTING_AUTO_SAVE_INTERVAL,
    SETTING_WRITER_DEFAULT_DOC_FORMAT,
    SETTING_IMPRESS_DEFAULT_DOC_FORMAT,
    SETTING_CALC_DEFAULT_DOC_FORMAT,

#endif // ENABLE_LOCKDOWN

    SETTINGS_LAST
};

struct ConfigurationValue
{
    const ConfigurationSetting nSettingId;
    const gchar *GconfItem;
    const char *OOoConfItem;
    const char *OOoConfValueType;
    const sal_Bool bLocked;
    const sal_Bool bNeedsTranslation;
    const ConfigurationSetting nDependsOn;
};

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;
namespace util = css::util ;

/**
  Implementation of the XLayer interface for the Gconf values mapped into
  the org.openoffice.* configuration component.
  */
class GconfLayer : public cppu::WeakImplHelper2<backend::XLayer, util::XTimeStamped>
{
public :
    /**
      Constructor given the component context

      @param xContext       The component context
    */

    GconfLayer( const uno::Reference<uno::XComponentContext>& xContext,
                const ConfigurationValue pConfigurationValuesList[],
                const sal_Int32 nConfigurationValues,
                const char * pPreloadValuesList[] );

    // XLayer
    virtual void SAL_CALL readData(
        const uno::Reference<backend::XLayerHandler>& xHandler)
        throw ( backend::MalformedDataException,
                lang::NullPointerException,
                lang::WrappedTargetException,
                uno::RuntimeException );

    // XTimeStamped
    virtual rtl::OUString SAL_CALL getTimestamp(void)
            throw (uno::RuntimeException);

    protected:

    /** Destructor */
    ~GconfLayer(void) {}

private :
    uno::Reference<backend::XLayerContentDescriber> m_xLayerContentDescriber;
    const ConfigurationValue* m_pConfigurationValuesList;
    const sal_Int32 m_nConfigurationValues;
    const char** m_pPreloadValuesList;
  } ;

#endif // GCONFLAYER
