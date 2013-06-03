//
//  Client.m
//  sdremote
//
//  Created by Liu Siqi on 6/3/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "Client.h"
#import "Server.h"
#import "Receiver.h"
#import "CommunicationManager.h"

@interface Client()

@property (nonatomic, strong) NSInputStream* mInputStream;
@property (nonatomic, strong) NSOutputStream* mOutputStream;

@property (nonatomic, strong) NSString* mPin;
@property (nonatomic, strong) NSString* mName;
@property int mPort;

@property (nonatomic, weak) Server* mServer;
@property (nonatomic, weak) Receiver* mReceiver;
@property (nonatomic, weak) CommunicationManager* mComManager;

@end



@implementation Client

@synthesize mInputStream = _mInputStream;
@synthesize mOutputStream = _mOutputStream;
@synthesize mPin = _mPin;
@synthesize mName = _mName;
@synthesize mServer = _mServer;
@synthesize mComManager = _mComManager;

NSString * const CHARSET = @"UTF-8";

- (id) initWithServer:(Server*)server
           managedBy:(CommunicationManager*)manager
       interpretedBy:(Receiver*)receiver
{
    self.mPin = @"";
    self.mName = server.serverName;
    self.mComManager = manager;
    self.mReceiver = receiver;
    // hardcoded here to test the communication TODO
    self.mPort = 1599;
    
    return self;
}

- (void)streamOpenWithIp:(NSString *)ip withPortNumber:(int)portNumber
{
    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
    CFStreamCreatePairWithSocketToHost(kCFAllocatorDefault, (__bridge CFStringRef)ip, portNumber, &readStream, &writeStream);
    
    if(readStream && writeStream)
    {
        CFReadStreamSetProperty(readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        CFWriteStreamSetProperty(writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        
        //Setup mInputStream
        self.mInputStream = (__bridge NSInputStream *)readStream;
        [self.mInputStream setDelegate:self];
        [self.mInputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [self.mInputStream open];
        
        //Setup outputstream
        self.mOutputStream = (__bridge NSOutputStream *)writeStream;
        [self.mOutputStream setDelegate:self];
        [self.mOutputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [self.mOutputStream open];
    }
}


- (void) connect
{
    [self streamOpenWithIp:self.mServer.serverAddress withPortNumber:self.mPort];
    
}



@end
