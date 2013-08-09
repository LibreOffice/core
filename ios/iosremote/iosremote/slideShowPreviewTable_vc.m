/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "slideShowPreviewTable_vc.h"
#import "CommunicationManager.h"
#import "CommandTransmitter.h"
#import "CommandInterpreter.h"
#import "ControlVariables.h"
#import "SlideShow.h"

@interface slideShowPreviewTable_vc ()

@property (nonatomic, weak) UIButton * startButton;

@end

@implementation slideShowPreviewTable_vc

@synthesize startButton = _startButton;
@synthesize optionsTable = _optionsTable;
@synthesize optionsArray = _optionsArray;
@synthesize titleObserver = _titleObserver;

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        return YES;
    else {
        return toInterfaceOrientation == UIInterfaceOrientationMaskPortrait;
    }
}

- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void) viewDidDisappear:(BOOL)animated
{
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowStartObserver];
    self.slideShowStartObserver = nil;
    [super viewDidDisappear:animated];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.optionsArray count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"optionCell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier];
        UISwitch *toggleSwitch = [[UISwitch alloc] init];
        cell.accessoryView = [[UIView alloc] initWithFrame:toggleSwitch.frame];
        if (indexPath.row == 0) {
            [toggleSwitch setOn:[[NSUserDefaults standardUserDefaults] boolForKey:KEY_TIMER]];
        } else {
            [toggleSwitch setOn:[[NSUserDefaults standardUserDefaults] boolForKey:KEY_POINTER]];
        }
        [cell.accessoryView addSubview:toggleSwitch];
    }
    cell.textLabel.text = [self.optionsArray objectAtIndex:indexPath.row];
    
    return cell;
}


-(IBAction)startPresentationAction:(id)sender {
    for (UITableViewCell *cell in self.tableView.visibleCells) {
        UISwitch * toggle = [[[cell accessoryView] subviews] objectAtIndex:0];

        if ([cell.textLabel.text isEqualToString:OPTION_TIMER])
            [[NSUserDefaults standardUserDefaults] setBool:[toggle isOn] forKey:KEY_TIMER];
        else if ([cell.textLabel.text isEqualToString:OPTION_POINTER])
            [[NSUserDefaults standardUserDefaults] setBool:[toggle isOn] forKey:KEY_POINTER];
    }
    [[self.comManager transmitter] startPresentation];
}

- (UIButton *)startButton{
    if (_startButton == nil) {
        _startButton = [UIButton buttonWithType:UIButtonTypeCustom];
        [_startButton setBackgroundImage:[UIImage imageNamed:@"navBarButtonNormal"] forState:UIControlStateNormal];
        if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) {
            _startButton.frame = CGRectMake(10.0, 30.0, 300.0, 50.0);
        } else {
            self.modalViewController.view.backgroundColor = [UIColor clearColor];
            _startButton.frame = CGRectMake(30.0, 50.0, 470.0, 50.0);
        }
        [_startButton setTitle:@"Start Presentation" forState:UIControlStateNormal];
        [_startButton setTitleColor:kTintColor forState:UIControlStateNormal];
        [_startButton setTitleColor:[UIColor lightGrayColor] forState:UIControlStateHighlighted];
        [_startButton addTarget:self action:@selector(startPresentationAction:) forControlEvents:UIControlEventTouchUpInside];
        
        _startButton.tag = 1;
    }
    return _startButton;
}

- (UIView *)tableView:(UITableView *)tableView viewForFooterInSection:(NSInteger)section
{
    UIView* customView;
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) {
        customView = [[UIView alloc] initWithFrame:CGRectMake(0.0, 0.0, 320.0, 100.0)];
    } else if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        customView = [[UIView alloc] initWithFrame:CGRectMake(0.0, 0.0, 540.0, 100.0)];
    [customView addSubview:self.startButton];
    
    customView.center = CGPointMake(tableView.center.x, customView.center.y);
    self.startButton.center = customView.center;
    return customView;
}

- (CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section
{
    return 100.0;
}

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell * cell = [tableView cellForRowAtIndexPath:indexPath];
    UISwitch * toggle = [[[cell accessoryView] subviews] objectAtIndex:0];
    [toggle setOn:![toggle isOn] animated:YES];
    [cell setSelected:NO animated:YES];
}

- (void)viewDidUnload {
    [self setOptionsTable:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowStartObserver];
    self.slideShowStartObserver = nil;
    [super viewDidUnload];
}
@end
