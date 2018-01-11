//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit
import Foundation


// AppDelegate is a Delegate class that receives calls from the iOS
// kernel, and theirby allows stop/start/sleep of the application
@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate
{
    var window: UIWindow?



    // sent when clicking on a OO document in another app
    // allowing this app to handle the document.
    // remark if the app is not started it will be started first
    func application(_ app: UIApplication,
                     open url: URL, options: [UIApplicationOpenURLOptionsKey : Any] = [:])
        -> Bool
    {
        let document = window?.rootViewController?.childViewControllers[0] as! DocumentController
        document.doOpen(url)
        return true
    }



    // this function is called when the app is first started (loaded from EEProm)
    // it initializes the LO system and prepares for a normal run
    func application(_ application: UIApplication,
                     didFinishLaunchingWithOptions launchOptions:
                         [UIApplicationLaunchOptionsKey: Any]?)
        -> Bool
    {
        // Get version info
        let appInfo = Bundle.main.infoDictionary! as Dictionary<String,AnyObject>
        let applicationVersion = (appInfo["CFBundleShortVersionString"] as! String) + "." +
                                 (appInfo["CFBundleVersion"] as! String)

        // Add version string to setting
        let defaults = UserDefaults.standard
        defaults.set(applicationVersion, forKey: "application_version")
        defaults.synchronize()

        // start LibreOfficeKit
        let _ = LOKitThread.instance

        return true
    }



    // Sent when the application is about to move from active to inactive state.
    // This can occur for certain types of temporary interruptions
    // (such as an incoming phone call or SMS message)
    // or when the user quits the application and it begins the transition
    // jto the background state.
    // Use this method to pause ongoing tasks, disable timers,
    // and invalidate graphics rendering callbacks.
    func applicationWillResignActive(_ application: UIApplication)
    {
        // NOT used in this App
    }



    // Sent when the application enters background (hipernating)
    // Use this method to release shared resources, save user data, invalidate timers,
    // and store enough application state information to restore your application
    // to its current state jin case it is terminated later.
    // If your application supports background execution,
    // this method is called instead of applicationWillTerminate: when the user quits.
    func applicationDidEnterBackground(_ application: UIApplication)
    {
        let document = window?.rootViewController?.childViewControllers[0]
            as! DocumentController
        document.Hipernate()
    }



    // Sent before the application reenters foreground (hipernating -> active)
    // Restart timers, tasks as well as graphic rendering
    func applicationWillEnterForeground(_ application: UIApplication)
    {
        let document = window?.rootViewController?.childViewControllers[0] as! DocumentController
        document.LeaveHipernate()
    }



    // Sent after the application reenters foreground (hipernating -> active)
    // Restart timers, tasks as well as graphic rendering
    func applicationDidBecomeActive(_ application: UIApplication)
    {
        // NOT used in this App
    }



    // Sent when the application is about to terminate. Save data if appropriate.
    // See also applicationDidEnterBackground:.
    // Saves changes in the application's managed object context before the application terminates.
    func applicationWillTerminate(_ application: UIApplication)
    {
        let document = window?.rootViewController?.childViewControllers[0] as! DocumentController
        document.Hipernate()
    }
}

