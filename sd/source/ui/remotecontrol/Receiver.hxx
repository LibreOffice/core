
#ifndef _SD_IMPRESSREMOTE_RECEIVER_HXX
#define _SD_IMPRESSREMOTE_RECEIVER_HXX

#include <com/sun/star/presentation/XSlideShowController.hpp>



#include <stdlib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>

// Library_sd.mk:
// $(eval $(call gb_Library_use_externals,sd,\
//     gobject \
// ))


namespace sd
{
    
class Receiver
{
public:
    Receiver();
    ~Receiver();
    void parseCommand(char* aCommand, XSlideShowController *aController);    
    
private:
    

}

}

#endif // _SD_IMPRESSREMOTE_RECEIVER_HXX