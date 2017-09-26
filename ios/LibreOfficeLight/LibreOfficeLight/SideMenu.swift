//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit


class SidebarController: UIViewController
{
    // Temp. functions
    @IBAction func doStyle1(_ sender: UIButton)
    {
        loadController("Document")
    }



    @IBAction func doStyle2(_ sender: UIButton)
    {
        loadController("Document")
    }



    @IBAction func doStyle3(_ sender: UIButton)
    {
        loadController("Document")
    }



    @IBAction func doStyle4(_ sender: UIButton)
    {
        loadController("Document")
    }



    @IBAction func doStyle5(_ sender: UIButton)
    {
        loadController("Document")
    }



    @IBAction func doStyle6(_ sender: UIButton)
    {
        loadController("Document")
    }


    @IBAction func doStyle7(_ sender: UIButton)
    {
        loadController("Document")
    }



    func loadController(_ name:String)
    {
        let destController : UIViewController = self.storyboard!.instantiateViewController(withIdentifier: name)
        self.navigationController!.pushViewController(destController, animated: true)

        UIView.animate(withDuration: 0.3, animations: { () -> Void in
            self.view.frame = CGRect(x: -UIScreen.main.bounds.size.width, y: 0, width: UIScreen.main.bounds.size.width,height: UIScreen.main.bounds.size.height)
            self.view.layoutIfNeeded()
            self.view.backgroundColor = UIColor.clear
        }, completion: { (finished) -> Void in
            self.view.removeFromSuperview()
            self.removeFromParentViewController()
        })
    }



    override func viewDidLoad()
    {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
    }



    override func didReceiveMemoryWarning()
    {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }



    override func viewWillAppear(_ animated: Bool)
    {
        super.viewWillAppear(animated)
    }
}
