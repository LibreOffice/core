//
//  IASKAppSettingsViewController.m
//  http://www.inappsettingskit.com
//
//  Copyright (c) 2009-2010:
//  Luc Vandal, Edovia Inc., http://www.edovia.com
//  Ortwin Gentz, FutureTap GmbH, http://www.futuretap.com
//  All rights reserved.
//
//  It is appreciated but not required that you give credit to Luc Vandal and Ortwin Gentz,
//  as the original authors of this code. You can give credit in a blog post, a tweet or on
//  a info page of your app. Also, the original authors appreciate letting them know if you use this code.
//
//  This code is licensed under the BSD license that is available at: http://www.opensource.org/licenses/bsd-license.php
//


#import "IASKAppSettingsViewController.h"
#import "IASKSettingsReader.h"
#import "IASKSettingsStoreUserDefaults.h"
#import "IASKPSToggleSwitchSpecifierViewCell.h"
#import "IASKPSSliderSpecifierViewCell.h"
#import "IASKPSTextFieldSpecifierViewCell.h"
#import "IASKPSTitleValueSpecifierViewCell.h"
#import "IASKSwitch.h"
#import "IASKSlider.h"
#import "IASKSpecifier.h"
#import "IASKSpecifierValuesViewController.h"
#import "IASKTextField.h"

static const CGFloat KEYBOARD_ANIMATION_DURATION = 0.3;
static const CGFloat MINIMUM_SCROLL_FRACTION = 0.2;
static const CGFloat MAXIMUM_SCROLL_FRACTION = 0.8;

static NSString *kIASKCredits = @"Powered by InAppSettingsKit"; // Leave this as-is!!!

#define kIASKSpecifierValuesViewControllerIndex       0
#define kIASKSpecifierChildViewControllerIndex        1

#define kIASKCreditsViewWidth                         285

CGRect IASKCGRectSwap(CGRect rect);

@interface IASKAppSettingsViewController ()
@property (nonatomic, strong) NSMutableArray *viewList;
@property (nonatomic, strong) id currentFirstResponder;

- (void) setup;

- (void)_textChanged:(id)sender;
- (void)synchronizeSettings;
- (void)userDefaultsDidChange;
- (void)reload;
@end

@implementation IASKAppSettingsViewController

@synthesize delegate = _delegate;
@synthesize viewList = _viewList;
@synthesize settingsReader = _settingsReader;
@synthesize file = _file;
@synthesize currentFirstResponder = _currentFirstResponder;
@synthesize showCreditsFooter = _showCreditsFooter;
@synthesize showDoneButton = _showDoneButton;
@synthesize settingsStore = _settingsStore;

#pragma mark accessors
- (IASKSettingsReader*)settingsReader {
	if (!_settingsReader) {
		_settingsReader = [[IASKSettingsReader alloc] initWithFile:self.file];
	}
	return _settingsReader;
}

- (id<IASKSettingsStore>)settingsStore {
	if (!_settingsStore) {
		_settingsStore = [[IASKSettingsStoreUserDefaults alloc] init];
	}
	return _settingsStore;
}

- (NSString*)file {
	if (!_file) {
		return @"Root";
	}
	return _file;
}

- (void)setFile:(NSString *)file {
	if (file != _file) {

		_file = [file copy];
	}

    self.tableView.contentOffset = CGPointMake(0, 0);
	self.settingsReader = nil; // automatically initializes itself
	[self.tableView reloadData];
}

- (BOOL)isPad {
	BOOL isPad = NO;
#if (__IPHONE_OS_VERSION_MAX_ALLOWED >= 30200)
	isPad = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad;
#endif
	return isPad;
}

#pragma mark standard view controller methods
- (id)init {
    return [self initWithNibName:@"IASKAppSettingsView" bundle:nil];
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // If set to YES, will add a DONE button at the right of the navigation bar
      _showDoneButton = YES;

      [self setup];
    }
    return self;
}

- (void)awakeFromNib {
	// If set to YES, will add a DONE button at the right of the navigation bar
	// if loaded via NIB, it's likely we sit in a TabBar- or NavigationController
	// and thus don't need the Done button
	_showDoneButton = NO;

  [self setup];
}

//common (NIB & code based) initialization
- (void) setup {
  // If set to YES, will display credits for InAppSettingsKit creators
  _showCreditsFooter = YES;
}

- (NSMutableArray *)viewList {
    if (!_viewList) {
		_viewList = [[NSMutableArray alloc] init];
		[_viewList addObject:[NSDictionary dictionaryWithObjectsAndKeys:@"IASKSpecifierValuesView", @"ViewName",nil]];
		[_viewList addObject:[NSDictionary dictionaryWithObjectsAndKeys:@"IASKAppSettingsView", @"ViewName",nil]];
	}
	return _viewList;
}

- (void) viewDidLoad {
  [super viewDidLoad];
  if ([self isPad]) {
    self.tableView.separatorStyle = UITableViewCellSeparatorStyleSingleLineEtched;
  }

    UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Done", nil) style:UIBarButtonItemStyleBordered target:self action:@selector(handleBack)];
    [backButton setBackgroundImage:[UIImage imageNamed:@"backButton"] forState:UIControlStateNormal barMetrics:UIBarMetricsDefault];
    self.navigationItem.leftBarButtonItem = backButton;
    
    self.tableView.backgroundView.backgroundColor = [UIColor colorWithRed:.968627451 green:.968627451 blue:.968627451 alpha:1.0];
}

- (void)viewDidUnload {
  [super viewDidUnload];

	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
	self.view = nil;
	self.viewList = nil;
}

- (void)viewWillAppear:(BOOL)animated {
	// if there's something selected, the value might have changed
	// so reload that row
	NSIndexPath *selectedIndexPath = [self.tableView indexPathForSelectedRow];
	if(selectedIndexPath) {
		[self.tableView reloadRowsAtIndexPaths:[NSArray arrayWithObject:selectedIndexPath]
							  withRowAnimation:UITableViewRowAnimationNone];
		// and reselect it, so we get the nice default deselect animation from UITableViewController
		[self.tableView selectRowAtIndexPath:selectedIndexPath animated:NO scrollPosition:UITableViewScrollPositionNone];
	}

	self.navigationItem.rightBarButtonItem = nil;
	self.navigationController.delegate = self;
	if (_showDoneButton) {
		UIBarButtonItem *buttonItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone
																					target:self
																					action:@selector(dismiss:)];
		self.navigationItem.rightBarButtonItem = buttonItem;
	}
	if (!self.title) {
		self.title = NSLocalizedString(@"Settings", @"");
	}

	if ([self.settingsStore isKindOfClass:[IASKSettingsStoreUserDefaults class]]) {
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(userDefaultsDidChange)
													 name:NSUserDefaultsDidChangeNotification
												   object:[NSUserDefaults standardUserDefaults]];
		[self userDefaultsDidChange]; // force update in case of changes while we were hidden
	}
	[super viewWillAppear:animated];
}

- (CGSize)contentSizeForViewInPopover {
    return [[self view] sizeThatFits:CGSizeMake(320, 2000)];
}

- (void)viewDidAppear:(BOOL)animated {
	[super viewDidAppear:animated];

	NSNotificationCenter *dc = [NSNotificationCenter defaultCenter];
	IASK_IF_IOS4_OR_GREATER([dc addObserver:self selector:@selector(synchronizeSettings) name:UIApplicationDidEnterBackgroundNotification object:[UIApplication sharedApplication]];);
	IASK_IF_IOS4_OR_GREATER([dc addObserver:self selector:@selector(reload) name:UIApplicationWillEnterForegroundNotification object:[UIApplication sharedApplication]];);
	[dc addObserver:self selector:@selector(synchronizeSettings) name:UIApplicationWillTerminateNotification object:[UIApplication sharedApplication]];
}

- (void)viewWillDisappear:(BOOL)animated {
	[NSObject cancelPreviousPerformRequestsWithTarget:self];
	[super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated {
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	if (!self.navigationController.delegate) {
		// hide the keyboard when we're popping from the navigation controller
		[self.currentFirstResponder resignFirstResponder];
	}

	[super viewDidDisappear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];

	// Release any cached data, images, etc that aren't in use.
}

- (void)navigationController:(UINavigationController *)navigationController willShowViewController:(UIViewController *)viewController animated:(BOOL)animated {
	if (![viewController isKindOfClass:[IASKAppSettingsViewController class]] && ![viewController isKindOfClass:[IASKSpecifierValuesViewController class]]) {
		[self dismiss:nil];
	}
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    _viewList = nil;
	_file = nil;
	_currentFirstResponder = nil;
	_settingsReader = nil;
    _settingsStore = nil;

	_delegate = nil;

}


#pragma mark -
#pragma mark Actions

- (IBAction)dismiss:(id)sender {
	[self.settingsStore synchronize];
	self.navigationController.delegate = nil;

	if (self.delegate && [self.delegate conformsToProtocol:@protocol(IASKSettingsDelegate)]) {
		[self.delegate settingsViewControllerDidEnd:self];
	}
}

- (void)toggledValue:(id)sender {
    IASKSwitch *toggle    = (IASKSwitch*)sender;
    IASKSpecifier *spec   = [_settingsReader specifierForKey:[toggle key]];

    if ([toggle isOn]) {
        if ([spec trueValue] != nil) {
            [self.settingsStore setObject:[spec trueValue] forKey:[toggle key]];
        }
        else {
            [self.settingsStore setBool:YES forKey:[toggle key]];
        }
    }
    else {
        if ([spec falseValue] != nil) {
            [self.settingsStore setObject:[spec falseValue] forKey:[toggle key]];
        }
        else {
            [self.settingsStore setBool:NO forKey:[toggle key]];
        }
    }
    [[NSNotificationCenter defaultCenter] postNotificationName:kIASKAppSettingChanged
                                                        object:[toggle key]
                                                      userInfo:[NSDictionary dictionaryWithObject:[self.settingsStore objectForKey:[toggle key]]
                                                                                           forKey:[toggle key]]];
}

- (void)sliderChangedValue:(id)sender {
    IASKSlider *slider = (IASKSlider*)sender;
    [self.settingsStore setFloat:[slider value] forKey:[slider key]];
    [[NSNotificationCenter defaultCenter] postNotificationName:kIASKAppSettingChanged
                                                        object:[slider key]
                                                      userInfo:[NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:[slider value]]
                                                                                           forKey:[slider key]]];
}


#pragma mark -
#pragma mark UITableView Functions

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
	return [self.settingsReader numberOfSections];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self.settingsReader numberOfRowsForSection:section];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    IASKSpecifier *specifier  = [self.settingsReader specifierForIndexPath:indexPath];
    if ([[specifier type] isEqualToString:kIASKCustomViewSpecifier]) {
		if ([self.delegate respondsToSelector:@selector(tableView:heightForSpecifier:)]) {
			return [self.delegate tableView:tableView heightForSpecifier:specifier];
		} else {
			return 0;
		}
	}
	return tableView.rowHeight;
}

- (NSString *)tableView:(UITableView*)tableView titleForHeaderInSection:(NSInteger)section {
    NSString *header = [self.settingsReader titleForSection:section];
	if (0 == header.length) {
		return nil;
	}
	return header;
}

- (UIView *)tableView:(UITableView*)tableView viewForHeaderInSection:(NSInteger)section {
	if ([self.delegate respondsToSelector:@selector(settingsViewController:tableView:viewForHeaderForSection:)]) {
		return [self.delegate settingsViewController:self tableView:tableView viewForHeaderForSection:section];
	} else {
		return nil;
	}
}

- (CGFloat)tableView:(UITableView*)tableView heightForHeaderInSection:(NSInteger)section {
	if ([self tableView:tableView viewForHeaderInSection:section] && [self.delegate respondsToSelector:@selector(settingsViewController:tableView:heightForHeaderForSection:)]) {
		CGFloat result;
		if ((result = [self.delegate settingsViewController:self tableView:tableView heightForHeaderForSection:section])) {
			return result;
		}

	}
	NSString *title;
	if ((title = [self tableView:tableView titleForHeaderInSection:section])) {
		CGSize size = [title sizeWithFont:[UIFont boldSystemFontOfSize:[UIFont labelFontSize]]
						constrainedToSize:CGSizeMake(tableView.frame.size.width - 2*kIASKHorizontalPaddingGroupTitles, INFINITY)
							lineBreakMode:UILineBreakModeWordWrap];
		return size.height+kIASKVerticalPaddingGroupTitles;
	}
	return 0;
}

- (NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section
{
	NSString *footerText = [self.settingsReader footerTextForSection:section];
	if (_showCreditsFooter && (section == [self.settingsReader numberOfSections]-1)) {
		// show credits since this is the last section
		if ((footerText == nil) || ([footerText length] == 0)) {
			// show the credits on their own
			return kIASKCredits;
		} else {
			// show the credits below the app's FooterText
			return [NSString stringWithFormat:@"%@\n\n%@", footerText, kIASKCredits];
		}
	} else {
		if ([footerText length] == 0) {
			return nil;
		}
		return [self.settingsReader footerTextForSection:section];
	}
}

- (UITableViewCell*)dequeueReusableCellWithIdentifier:(NSString*)identifier {
	UITableViewCell *cell = nil;
	if ([identifier isEqualToString:kIASKPSToggleSwitchSpecifier]) {
		cell = [[[NSBundle mainBundle] loadNibNamed:@"IASKPSToggleSwitchSpecifierViewCell"
											  owner:self
											options:nil] objectAtIndex:0];
	}
	else if ([identifier isEqualToString:kIASKPSMultiValueSpecifier] || [identifier isEqualToString:kIASKPSTitleValueSpecifier]) {
		cell = [[IASKPSTitleValueSpecifierViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:identifier];
		cell.accessoryType = [identifier isEqualToString:kIASKPSMultiValueSpecifier] ? UITableViewCellAccessoryDisclosureIndicator : UITableViewCellAccessoryNone;
	}
	else if ([identifier isEqualToString:kIASKPSTextFieldSpecifier]) {
		cell = (IASKPSTextFieldSpecifierViewCell*) [[[NSBundle mainBundle] loadNibNamed:@"IASKPSTextFieldSpecifierViewCell"
																				  owner:self
																				options:nil] objectAtIndex:0];
	}
	else if ([identifier isEqualToString:kIASKPSSliderSpecifier]) {
		cell = (IASKPSSliderSpecifierViewCell*) [[[NSBundle mainBundle] loadNibNamed:@"IASKPSSliderSpecifierViewCell"
																			   owner:self
																			 options:nil] objectAtIndex:0];
	} else if ([identifier isEqualToString:kIASKPSChildPaneSpecifier]) {
		cell = [[IASKPSTitleValueSpecifierViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:identifier];
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
	} else if ([identifier isEqualToString:kIASKButtonSpecifier]) {
		cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:identifier];
	} else if ([identifier isEqualToString:kIASKMailComposeSpecifier]) {
		cell = [[IASKPSTitleValueSpecifierViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:identifier];
		[cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
	} else {
		cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:identifier];
	}
	return cell;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	IASKSpecifier *specifier  = [self.settingsReader specifierForIndexPath:indexPath];
	if ([specifier.type isEqualToString:kIASKCustomViewSpecifier] && [self.delegate respondsToSelector:@selector(tableView:cellForSpecifier:)]) {
		UITableViewCell* cell = [self.delegate tableView:tableView cellForSpecifier:specifier];
		assert(nil != cell && "delegate must return a UITableViewCell for custom cell types");
		return cell;
	}

	UITableViewCell *cell = [self dequeueReusableCellWithIdentifier:specifier.type];

	if ([specifier.type isEqualToString:kIASKPSToggleSwitchSpecifier]) {
		((IASKPSToggleSwitchSpecifierViewCell*)cell).label.text = specifier.title;

		id currentValue = [self.settingsStore objectForKey:specifier.key];
		BOOL toggleState;
		if (currentValue) {
			if ([currentValue isEqual:specifier.trueValue]) {
				toggleState = YES;
			} else if ([currentValue isEqual:specifier.falseValue]) {
				toggleState = NO;
			} else {
				toggleState = [currentValue boolValue];
			}
		} else {
			toggleState = specifier.defaultBoolValue;
		}
		((IASKPSToggleSwitchSpecifierViewCell*)cell).toggle.on = toggleState;

		[((IASKPSToggleSwitchSpecifierViewCell*)cell).toggle addTarget:self action:@selector(toggledValue:) forControlEvents:UIControlEventValueChanged];
		((IASKPSToggleSwitchSpecifierViewCell*)cell).toggle.key = specifier.key;
	}
	else if ([specifier.type isEqualToString:kIASKPSMultiValueSpecifier]) {
		cell.textLabel.text = specifier.title;
		cell.detailTextLabel.text = [[specifier titleForCurrentValue:[self.settingsStore objectForKey:specifier.key] != nil ?
									  [self.settingsStore objectForKey:specifier.key] : specifier.defaultValue] description];
	}
	else if ([specifier.type isEqualToString:kIASKPSTitleValueSpecifier]) {
		cell.textLabel.text = specifier.title;
		id value = [self.settingsStore objectForKey:specifier.key] ? : specifier.defaultValue;

		NSString *stringValue;
		if (specifier.multipleValues || specifier.multipleTitles) {
			stringValue = [specifier titleForCurrentValue:value];
		} else {
			stringValue = [value description];
		}

		cell.detailTextLabel.text = stringValue;
		cell.userInteractionEnabled = NO;
	}
	else if ([specifier.type isEqualToString:kIASKPSTextFieldSpecifier]) {
		((IASKPSTextFieldSpecifierViewCell*)cell).label.text = specifier.title;

		NSString *textValue = [self.settingsStore objectForKey:specifier.key] != nil ? [self.settingsStore objectForKey:specifier.key] : specifier.defaultStringValue;
		if (textValue && ![textValue isMemberOfClass:[NSString class]]) {
			textValue = [NSString stringWithFormat:@"%@", textValue];
		}
		IASKTextField *textField = ((IASKPSTextFieldSpecifierViewCell*)cell).textField;
		textField.text = textValue;
		textField.key = specifier.key;
		textField.delegate = self;
		textField.secureTextEntry = [specifier isSecure];
		textField.keyboardType = specifier.keyboardType;
		textField.autocapitalizationType = specifier.autocapitalizationType;
		[textField addTarget:self action:@selector(_textChanged:) forControlEvents:UIControlEventEditingChanged];
		if([specifier isSecure]){
			textField.autocorrectionType = UITextAutocorrectionTypeNo;
		} else {
			textField.autocorrectionType = specifier.autoCorrectionType;
		}
		[cell setNeedsLayout];
	}
	else if ([specifier.type isEqualToString:kIASKPSSliderSpecifier]) {
		if (specifier.minimumValueImage.length > 0) {
			((IASKPSSliderSpecifierViewCell*)cell).minImage.image = [UIImage imageWithContentsOfFile:[_settingsReader pathForImageNamed:specifier.minimumValueImage]];
		}

		if (specifier.maximumValueImage.length > 0) {
			((IASKPSSliderSpecifierViewCell*)cell).maxImage.image = [UIImage imageWithContentsOfFile:[_settingsReader pathForImageNamed:specifier.maximumValueImage]];
		}

		IASKSlider *slider = ((IASKPSSliderSpecifierViewCell*)cell).slider;
		slider.minimumValue = specifier.minimumValue;
		slider.maximumValue = specifier.maximumValue;
		slider.value =	[self.settingsStore objectForKey:specifier.key] != nil ? [[self.settingsStore objectForKey:specifier.key] floatValue] : [specifier.defaultValue floatValue];
		[slider addTarget:self action:@selector(sliderChangedValue:) forControlEvents:UIControlEventValueChanged];
		slider.key = specifier.key;
		[cell setNeedsLayout];
	}
	else if ([specifier.type isEqualToString:kIASKPSChildPaneSpecifier]) {
		cell.textLabel.text = specifier.title;
	} else if ([specifier.type isEqualToString:kIASKOpenURLSpecifier] || [specifier.type isEqualToString:kIASKMailComposeSpecifier]) {
		cell.textLabel.text = specifier.title;
		cell.detailTextLabel.text = [specifier.defaultValue description];
	} else if ([specifier.type isEqualToString:kIASKButtonSpecifier]) {
		NSString *value = [self.settingsStore objectForKey:specifier.key];
		cell.textLabel.text = [value isKindOfClass:[NSString class]] ? [self.settingsReader titleForStringId:value] : specifier.title;
		cell.textLabel.textAlignment = UITextAlignmentCenter;
	} else {
		cell.textLabel.text = specifier.title;
	}
	return cell;
}

- (NSIndexPath *)tableView:(UITableView *)tableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	//create a set of specifier types that can't be selected
	static NSSet* noSelectionTypes = nil;
	if(nil == noSelectionTypes) {
		noSelectionTypes = [NSSet setWithObjects:kIASKPSToggleSwitchSpecifier, kIASKPSSliderSpecifier, nil];
	}

	IASKSpecifier *specifier  = [self.settingsReader specifierForIndexPath:indexPath];
	if([noSelectionTypes containsObject:specifier.type]) {
		return nil;
	} else {
		return indexPath;
	}
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    IASKSpecifier *specifier  = [self.settingsReader specifierForIndexPath:indexPath];

    //switches and sliders can't be selected (should be captured by tableView:willSelectRowAtIndexPath: delegate method)
    assert(![[specifier type] isEqualToString:kIASKPSToggleSwitchSpecifier]);
    assert(![[specifier type] isEqualToString:kIASKPSSliderSpecifier]);

    if ([[specifier type] isEqualToString:kIASKPSMultiValueSpecifier]) {
        IASKSpecifierValuesViewController *targetViewController = [[self.viewList objectAtIndex:kIASKSpecifierValuesViewControllerIndex] objectForKey:@"viewController"];

        if (targetViewController == nil) {
            // the view controller has not been created yet, create it and set it to our viewList array
            // create a new dictionary with the new view controller
            NSMutableDictionary *newItemDict = [NSMutableDictionary dictionaryWithCapacity:3];
            [newItemDict addEntriesFromDictionary: [self.viewList objectAtIndex:kIASKSpecifierValuesViewControllerIndex]];	// copy the title and explain strings

            targetViewController = [[IASKSpecifierValuesViewController alloc] initWithNibName:@"IASKSpecifierValuesView" bundle:nil];
            // add the new view controller to the dictionary and then to the 'viewList' array
            [newItemDict setObject:targetViewController forKey:@"viewController"];
            [self.viewList replaceObjectAtIndex:kIASKSpecifierValuesViewControllerIndex withObject:newItemDict];

            // load the view controll back in to push it
            targetViewController = [[self.viewList objectAtIndex:kIASKSpecifierValuesViewControllerIndex] objectForKey:@"viewController"];
        }
        [targetViewController setCurrentSpecifier:specifier];
        targetViewController.settingsReader = self.settingsReader;
        targetViewController.settingsStore = self.settingsStore;
        [[self navigationController] pushViewController:targetViewController animated:YES];
    }
    else if ([[specifier type] isEqualToString:kIASKPSTextFieldSpecifier]) {
		IASKPSTextFieldSpecifierViewCell *textFieldCell = (id)[tableView cellForRowAtIndexPath:indexPath];
		[textFieldCell.textField becomeFirstResponder];
    }
    else if ([[specifier type] isEqualToString:kIASKPSChildPaneSpecifier]) {


        Class vcClass = [specifier viewControllerClass];
        if (vcClass) {
            SEL initSelector = [specifier viewControllerSelector];
            if (!initSelector) {
                initSelector = @selector(init);
            }
            UIViewController * vc = [vcClass alloc];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
            [vc performSelector:initSelector withObject:[specifier file] withObject:[specifier key]];
#pragma clang diagnostic pop
			if ([vc respondsToSelector:@selector(setDelegate:)]) {
				[vc performSelector:@selector(setDelegate:) withObject:self.delegate];
			}
			if ([vc respondsToSelector:@selector(setSettingsStore:)]) {
				[vc performSelector:@selector(setSettingsStore:) withObject:self.settingsStore];
			}
			self.navigationController.delegate = nil;
            [self.navigationController pushViewController:vc animated:YES];

            return;
        }

        if (nil == [specifier file]) {
            [tableView deselectRowAtIndexPath:indexPath animated:YES];
            return;
        }

        IASKAppSettingsViewController *targetViewController = [[self.viewList objectAtIndex:kIASKSpecifierChildViewControllerIndex] objectForKey:@"viewController"];

        if (targetViewController == nil) {
            // the view controller has not been created yet, create it and set it to our viewList array
            // create a new dictionary with the new view controller
            NSMutableDictionary *newItemDict = [NSMutableDictionary dictionaryWithCapacity:3];
            [newItemDict addEntriesFromDictionary: [self.viewList objectAtIndex:kIASKSpecifierChildViewControllerIndex]];	// copy the title and explain strings

            targetViewController = [[[self class] alloc] initWithNibName:@"IASKAppSettingsView" bundle:nil];
			targetViewController.showDoneButton = NO;
			targetViewController.settingsStore = self.settingsStore;
			targetViewController.delegate = self.delegate;

            // add the new view controller to the dictionary and then to the 'viewList' array
            [newItemDict setObject:targetViewController forKey:@"viewController"];
            [self.viewList replaceObjectAtIndex:kIASKSpecifierChildViewControllerIndex withObject:newItemDict];

            // load the view controll back in to push it
            targetViewController = [[self.viewList objectAtIndex:kIASKSpecifierChildViewControllerIndex] objectForKey:@"viewController"];
        }
		targetViewController.file = specifier.file;
		targetViewController.title = specifier.title;
        targetViewController.showCreditsFooter = NO;
        [[self navigationController] pushViewController:targetViewController animated:YES];
    } else if ([[specifier type] isEqualToString:kIASKOpenURLSpecifier]) {
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
		[[UIApplication sharedApplication] openURL:[NSURL URLWithString:specifier.file]];
    } else if ([[specifier type] isEqualToString:kIASKButtonSpecifier]) {
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
		if ([self.delegate respondsToSelector:@selector(settingsViewController:buttonTappedForKey:)]) {
			[self.delegate settingsViewController:self buttonTappedForKey:[specifier key]];
		} else {
			// legacy code, provided for backward compatibility
			// the delegate mechanism above is much cleaner and doesn't leak
			Class buttonClass = [specifier buttonClass];
			SEL buttonAction = [specifier buttonAction];
			if ([buttonClass respondsToSelector:buttonAction]) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
                [buttonClass performSelector:buttonAction withObject:self withObject:[specifier key]];
#pragma clang diagnostic pop
				NSLog(@"InAppSettingsKit Warning: Using IASKButtonSpecifier without implementing the delegate method is deprecated");
			}
		}
    } else if ([[specifier type] isEqualToString:kIASKMailComposeSpecifier]) {
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
        if ([MFMailComposeViewController canSendMail]) {
            MFMailComposeViewController *mailViewController = [[MFMailComposeViewController alloc] init];
            mailViewController.navigationBar.barStyle = self.navigationController.navigationBar.barStyle;
			mailViewController.navigationBar.tintColor = self.navigationController.navigationBar.tintColor;

            if ([specifier localizedObjectForKey:kIASKMailComposeSubject]) {
                [mailViewController setSubject:[specifier localizedObjectForKey:kIASKMailComposeSubject]];
            }
            if ([[specifier specifierDict] objectForKey:kIASKMailComposeToRecipents]) {
                [mailViewController setToRecipients:[[specifier specifierDict] objectForKey:kIASKMailComposeToRecipents]];
            }
            if ([[specifier specifierDict] objectForKey:kIASKMailComposeCcRecipents]) {
                [mailViewController setCcRecipients:[[specifier specifierDict] objectForKey:kIASKMailComposeCcRecipents]];
            }
            if ([[specifier specifierDict] objectForKey:kIASKMailComposeBccRecipents]) {
                [mailViewController setBccRecipients:[[specifier specifierDict] objectForKey:kIASKMailComposeBccRecipents]];
            }
            if ([specifier localizedObjectForKey:kIASKMailComposeBody]) {
                BOOL isHTML = NO;
                if ([[specifier specifierDict] objectForKey:kIASKMailComposeBodyIsHTML]) {
                    isHTML = [[[specifier specifierDict] objectForKey:kIASKMailComposeBodyIsHTML] boolValue];
                }

              if ([self.delegate respondsToSelector:@selector(settingsViewController:mailComposeBodyForSpecifier:)]) {
                    [mailViewController setMessageBody:[self.delegate settingsViewController:self
                                                                 mailComposeBodyForSpecifier:specifier] isHTML:isHTML];
                }
                else {
                    [mailViewController setMessageBody:[specifier localizedObjectForKey:kIASKMailComposeBody] isHTML:isHTML];
                }
            }

            UIViewController<MFMailComposeViewControllerDelegate> *vc = nil;

          if ([self.delegate respondsToSelector:@selector(settingsViewController:viewControllerForMailComposeViewForSpecifier:)]) {
            vc = [self.delegate settingsViewController:self viewControllerForMailComposeViewForSpecifier:specifier];
            }

            if (vc == nil) {
                vc = self;
            }

            mailViewController.mailComposeDelegate = vc;
            [vc presentModalViewController:mailViewController animated:YES];
        } else {
            UIAlertView *alert = [[UIAlertView alloc]
                                  initWithTitle:NSLocalizedString(@"Mail not configured", @"InAppSettingsKit")
                                  message:NSLocalizedString(@"This device is not configured for sending Email. Please configure the Mail settings in the Settings app.", @"InAppSettingsKit")
                                  delegate: nil
                                  cancelButtonTitle:NSLocalizedString(@"OK", @"InAppSettingsKit")
                                  otherButtonTitles:nil];
            [alert show];
        }

	} else if ([[specifier type] isEqualToString:kIASKCustomViewSpecifier] && [self.delegate respondsToSelector:@selector(settingsViewController:tableView:didSelectCustomViewSpecifier:)]) {
        [self.delegate settingsViewController:self tableView:tableView didSelectCustomViewSpecifier:specifier];
    } else {
        [tableView deselectRowAtIndexPath:indexPath animated:NO];
    }
}


#pragma mark -
#pragma mark MFMailComposeViewControllerDelegate Function

-(void)mailComposeController:(MFMailComposeViewController*)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError*)error {

    // Forward the mail compose delegate
    if ([self.delegate respondsToSelector:@selector(settingsViewController:mailComposeController:didFinishWithResult:error:)]) {
         [self.delegate settingsViewController:self
                         mailComposeController:controller
                           didFinishWithResult:result
                                         error:error];
     }

    // NOTE: No error handling is done here
    [self dismissModalViewControllerAnimated:YES];
}

#pragma mark -
#pragma mark UITextFieldDelegate Functions

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField {
	self.currentFirstResponder = textField;
	return YES;
}

- (void)_textChanged:(id)sender {
    IASKTextField *text = (IASKTextField*)sender;
    [_settingsStore setObject:[text text] forKey:[text key]];
    [[NSNotificationCenter defaultCenter] postNotificationName:kIASKAppSettingChanged
                                                        object:[text key]
                                                      userInfo:[NSDictionary dictionaryWithObject:[text text]
                                                                                           forKey:[text key]]];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField{
	[textField resignFirstResponder];
	self.currentFirstResponder = nil;
	return YES;
}


#pragma mark Notifications

- (void)synchronizeSettings {
    [_settingsStore synchronize];
}

static NSDictionary *oldUserDefaults = nil;
- (void)userDefaultsDidChange {
	NSDictionary *currentDict = [NSUserDefaults standardUserDefaults].dictionaryRepresentation;
	NSMutableArray *indexPathsToUpdate = [NSMutableArray array];
	for (NSString *key in currentDict.allKeys) {
		if (![[oldUserDefaults valueForKey:key] isEqual:[currentDict valueForKey:key]]) {
			NSIndexPath *path = [self.settingsReader indexPathForKey:key];
			if (path && ![[self.settingsReader specifierForKey:key].type isEqualToString:kIASKCustomViewSpecifier]) {
				[indexPathsToUpdate addObject:path];
			}
		}
	}
	oldUserDefaults = currentDict;


	for (UITableViewCell *cell in self.tableView.visibleCells) {
		if ([cell isKindOfClass:[IASKPSTextFieldSpecifierViewCell class]] && [((IASKPSTextFieldSpecifierViewCell*)cell).textField isFirstResponder]) {
			[indexPathsToUpdate removeObject:[self.tableView indexPathForCell:cell]];
		}
	}
	if (indexPathsToUpdate.count) {
		[self.tableView reloadRowsAtIndexPaths:indexPathsToUpdate withRowAnimation:UITableViewRowAnimationNone];
	}
}

- (void)reload {
	// wait 0.5 sec until UI is available after applicationWillEnterForeground
	[self.tableView performSelector:@selector(reloadData) withObject:nil afterDelay:0.5];
}

#pragma mark CGRect Utility function
CGRect IASKCGRectSwap(CGRect rect) {
	CGRect newRect;
	newRect.origin.x = rect.origin.y;
	newRect.origin.y = rect.origin.x;
	newRect.size.width = rect.size.height;
	newRect.size.height = rect.size.width;
	return newRect;
}
@end
