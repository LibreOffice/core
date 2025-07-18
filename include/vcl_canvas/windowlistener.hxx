#pragma once

#include <com/sun/star/awt/WindowEvent.hpp>

namespace vcl_canvas
{
class WindowListener
{
public:
    virtual void windowResized( const ::css::awt::WindowEvent& e ) = 0;
    virtual void windowMoved( const ::css::awt::WindowEvent& e ) = 0;
    virtual void windowShown( const ::css::lang::EventObject& e ) = 0;
    virtual void windowHidden( const ::css::lang::EventObject& e ) = 0;
};
}
