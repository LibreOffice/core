#include "EventHandler.hxx"

namespace VLC
{
EventHandler::EventHandler( const char *name )
    : Thread( name )
{
}

void EventHandler::execute()
{
    TCallback callback;
    do
    {
        mCallbackQueue.pop( callback );

        if ( callback.empty() )
            return;
        else
            callback();
    } while ( true );
}

}
