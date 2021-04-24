#pragma once
#include <svx/svdobj.hxx>
#include <vector>
#include <functional>
namespace tools::silverdev
{
struct ViewUpdateEvent
{
};
struct PreviousZoomEvent
{
    SdrObject* zooming;
};
struct NextZoomEvent
{
    SdrObject* zooming;
};
// inspired by https://stackoverflow.com/a/59707355
class NotificationCenter
{
public:
    template <typename TEvent> void addListener(std::function<void(TEvent&)> callback);
    template <typename TEvent> void clearListeners();
    template <typename TEvent> void fireEvent(TEvent& event);

private:
    template <typename TEvent> std::vector<std::function<void(TEvent&)>>& getListeners();
};
class S
{
public:
    bool isFirstZoom;
    std::vector<SdrObject*>::iterator zoomIterator;
    std::vector<basegfx::B2DPoint>::iterator centerIterator;
    std::vector<double>::iterator requestedZoomIterator;
    bool hasZooms();
    // The object in zoomings have zoomTargetObject->GetCurrentBoundRect();
    std::vector<basegfx::B2DPoint> centersOfObjects;
    std::vector<double> requestedZooms;
    std::vector<SdrObject*> zoomings;
    NotificationCenter notificationCenter;
    S();
    S(S const&) = delete;
    void operator=(S const&) = delete;
};
extern S& getInstance2();
template <typename TEvent>
void NotificationCenter::addListener(std::function<void(TEvent&)> callback)
{
    getListeners<TEvent>().push_back(std::move(callback));
}
template <typename TEvent> void NotificationCenter::fireEvent(TEvent& event)
{
    for (auto& listener : getListeners<TEvent>())
    {
        listener(event);
    }
}
template <typename TEvent>
std::vector<std::function<void(TEvent&)>>& NotificationCenter::getListeners()
{
    static std::vector<std::function<void(TEvent&)>> listeners;
    return listeners;
}
template <typename TEvent> void NotificationCenter::clearListeners()
{
    getListeners<TEvent>().clear();
}
}