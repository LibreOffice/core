//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit
import Foundation



@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate
{
    var window: UIWindow?



    // MARK: - AppDelegate functions

    func application(_ app: UIApplication, open url: URL, options: [UIApplicationOpenURLOptionsKey : Any] = [:]) -> Bool
    {
        // called when started from another Application.
        return true
    }


    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplicationLaunchOptionsKey: Any]?) -> Bool
    {
        // Get version info
        let appInfo = Bundle.main.infoDictionary! as Dictionary<String,AnyObject>
        let applicationVersion = (appInfo["CFBundleShortVersionString"] as! String) + "." +
                                 (appInfo["CFBundleVersion"] as! String)
        let defaults = UserDefaults.standard
        defaults.set(applicationVersion, forKey: "application_version")
        defaults.synchronize()

        // start LibreOfficeKit
        BridgeLOkit_Init(Bundle.main.bundlePath)

        // Override point for customization after application launch.
        return true
    }



    func applicationWillResignActive(_ application: UIApplication)
    {
        // Sent when the application is about to move from active to inactive state.
        // This can occur for certain types of temporary interruptions
        // (such as an incoming phone call or SMS message)
        // or when the user quits the application and it begins the transition to the background state.
        // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks.
        // Games should use this method to pause the game.
    }



    func applicationDidEnterBackground(_ application: UIApplication)
    {
        // Use this method to release shared resources, save user data, invalidate timers,
        // and store enough application state information to restore your application to its current state
        // in case it is terminated later.
        // If your application supports background execution,
        // this method is called instead of applicationWillTerminate: when the user quits.
    }



    func applicationWillEnterForeground(_ application: UIApplication)
    {
        // Called as part of the transition from the background to the active state;
        // here you can undo many of the changes made on entering the background.

        //JIX add code to check iCloud again
    }



    func applicationDidBecomeActive(_ application: UIApplication)
    {
    }



    func applicationWillTerminate(_ application: UIApplication)
    {
        // Called when the application is about to terminate. Save data if appropriate.
        // See also applicationDidEnterBackground:.
        // Saves changes in the application's managed object context before the application terminates.
    }
}

