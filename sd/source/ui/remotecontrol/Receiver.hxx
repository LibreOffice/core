
#ifndef _SD_IMPRESSREMOTE_RECEIVER_HXX
#define _SD_IMPRESSREMOTE_RECEIVER_HXX

#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XPresentation.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>

#include <stdlib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>

using namespace com::sun::star::presentation;
using namespace com::sun::star::uno;
namespace sd
{

class Receiver
{
public:
    Receiver();
    ~Receiver();
    void parseCommand( const char* aCommand, sal_Int32 size, XSlideShowController *aController );

private:
    void executeCommand( JsonObject *aObject, Reference<XSlideShowController> aController );

};

}

#endif // _SD_IMPRESSREMOTE_RECEIVER_HXX
